#include "FXAAPass.h"

#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Presentation/PresentationService.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Math/Vec2f.h>
#include <Runtime/RHI/Command/RenderingDesc.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Pipeline/GraphicsPipelineDesc.h>
#include <Runtime/RHI/Sampler/SamplerDesc.h>
#include <Runtime/RHI/Shader/ShaderCompiler.h>
#include <Runtime/RHI/Shader/ShaderModuleDesc.h>
#include <Runtime/RHI/Texture/Texture.h>

namespace Horizon::Rendering::Pass
{
	namespace
	{
		struct FXAAConstants
		{
			Math::Vec2f rcpFrame;
			u32 textureIndex;
			u32 samplerIndex;
		};
	}

	b8 FXAAPass::OnInitialize()
	{
		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, FXAAPass cannot be created");
			return false;
		}

		RHI::Device* pDevice = pContext->GetDevice();

		RHI::ShaderCompileDesc vertexCompile = {};
		vertexCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/PlaneDraw.vert.hlsl";
		vertexCompile.entryPoint = "VSMain";
		vertexCompile.stage = RHI::ShaderStage::Vertex;
		vertexCompile.target = RHI::ShaderTarget(pDevice->GetBackend());
		List<u8> vertexCode = RHI::ShaderCompiler::Compile(vertexCompile);

		RHI::ShaderCompileDesc fragmentCompile = {};
		fragmentCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/FXAAPass.frag.hlsl";
		fragmentCompile.entryPoint = "PSMain";
		fragmentCompile.stage = RHI::ShaderStage::Pixel;
		fragmentCompile.target = RHI::ShaderTarget(pDevice->GetBackend());
		List<u8> fragmentCode = RHI::ShaderCompiler::Compile(fragmentCompile);

		RHI::ShaderModuleDesc vertexModule = {};
		vertexModule.stage = RHI::ShaderStage::Vertex;
		vertexModule.entryPoint = "VSMain";
		vertexModule.byteCode = vertexCode;
		m_vertexShader = pDevice->CreateRHIShaderModule(vertexModule);

		RHI::ShaderModuleDesc fragmentModule = {};
		fragmentModule.stage = RHI::ShaderStage::Pixel;
		fragmentModule.entryPoint = "PSMain";
		fragmentModule.byteCode = fragmentCode;
		m_fragmentShader = pDevice->CreateRHIShaderModule(fragmentModule);

		if (!m_vertexShader || !m_fragmentShader)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "FXAAPass shaders could not be created");
			return false;
		}

		RHI::ColorTarget colorTarget = {};
		colorTarget.format = Engine::PresentationService::SceneFormat;

		RHI::GraphicsPipelineDesc pipelineDesc = {};
		pipelineDesc.pLayout = pContext->GetGlobalLayout();
		pipelineDesc.pVertexShader = m_vertexShader.Get();
		pipelineDesc.pPixelShader = m_fragmentShader.Get();
		pipelineDesc.rasterizer.cullMode = RHI::CullMode::None;
		pipelineDesc.depthStencil.depthTest = false;
		pipelineDesc.depthStencil.depthWrite = false;
		pipelineDesc.colorTargets.PushBack(colorTarget);
		pipelineDesc.sampleCount = RHI::SampleCount::Count1;

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "FXAAPass pipeline could not be created");
			return false;
		}

		RHI::SamplerDesc samplerDesc = {};
		samplerDesc.mipFilter = RHI::SamplerFilter::Nearest;
		samplerDesc.addressU = RHI::SamplerAddressMode::ClampToEdge;
		samplerDesc.addressV = RHI::SamplerAddressMode::ClampToEdge;
		samplerDesc.addressW = RHI::SamplerAddressMode::ClampToEdge;
		m_sampler = pDevice->CreateRHISampler(samplerDesc);

		if (!m_sampler)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "FXAAPass sampler could not be created");
			return false;
		}

		m_resourceHeap = pContext->GetResourceHeap();
		m_sourceView = m_resourceHeap->Allocate();

		m_samplerHeap = pContext->GetSamplerHeap();
		m_samplerView = m_samplerHeap->Allocate();
		m_samplerHeap->Write(m_samplerView, m_sampler.Get());

		return true;
	}

	void FXAAPass::OnFinalize()
	{
		if (m_samplerView != kInvalid32)
		{
			m_samplerHeap->Free(m_samplerView);
			m_samplerView = kInvalid32;
		}

		if (m_sourceView != kInvalid32)
		{
			m_resourceHeap->Free(m_sourceView);
			m_sourceView = kInvalid32;
		}

		m_sampler.Reset();
		m_pipeline.Reset();
		m_fragmentShader.Reset();
		m_vertexShader.Reset();
	}

	void FXAAPass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
		graph.Reads<Resource::SceneColorResolved>(RHI::ResourceState::ShaderResource);
		graph.Writes<Resource::SceneColor>(RHI::ResourceState::RenderTarget);
	}

	void FXAAPass::Render(const Engine::Blackboard& board)
	{
		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pOutput = GetTexture<Resource::SceneColor>();

		m_resourceHeap->Write(m_sourceView, RHI::DescriptorType::Texture, GetTextureView<Resource::SceneColorResolved>());

		RHI::RenderingDesc desc = {};
		desc.colorAttachments[0].pView = GetTextureView<Resource::SceneColor>();
		desc.colorAttachments[0].loadOp = RHI::LoadOp::DontCare;
		desc.colorAttachments[0].storeOp = RHI::StoreOp::Store;
		desc.colorAttachmentCount = 1;

		pCmdList->StartRendering(desc);

		RHI::Viewport viewport = {};
		viewport.width = static_cast<f32>(pOutput->GetWidth());
		viewport.height = static_cast<f32>(pOutput->GetHeight());
		pCmdList->SetViewport(viewport);

		RHI::Scissor scissor = {};
		scissor.width = static_cast<i32>(pOutput->GetWidth());
		scissor.height = static_cast<i32>(pOutput->GetHeight());
		pCmdList->SetScissor(scissor);

		pCmdList->BindPipeline(m_pipeline.Get());

		FXAAConstants constants = {};
		constants.rcpFrame = Math::Vec2f(1.0f / static_cast<f32>(pOutput->GetWidth()), 1.0f / static_cast<f32>(pOutput->GetHeight()));
		constants.textureIndex = m_sourceView;
		constants.samplerIndex = m_samplerView;

		pCmdList->SetPushConstants(&constants, sizeof(FXAAConstants));
		pCmdList->Draw(3);

		pCmdList->EndRendering();
	}
}
