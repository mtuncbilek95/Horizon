#include "SkyCompositePass.h"

#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Presentation/PresentationService.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/ForwardPass.h>
#include <Engine/Rendering/Forward+/ProceduralSkyPass.h>
#include <Engine/Rendering/Forward+/ResolvePass.h>
#include <Engine/Rendering/Forward+/Resources.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/RHI/Command/RenderingDesc.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Pipeline/GraphicsPipelineDesc.h>
#include <Runtime/RHI/Shader/ShaderCompiler.h>
#include <Runtime/RHI/Shader/ShaderModuleDesc.h>
#include <Runtime/RHI/Texture/Texture.h>

namespace Horizon::Rendering::Pass
{
	namespace
	{
		struct SkyCompositeConstants
		{
			u32 skyTextureIndex;
		};
	}

	b8 SkyCompositePass::OnInitialize()
	{
		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, SkyCompositePass cannot be created");
			return false;
		}

		RHI::Device* pDevice = pContext->GetDevice();

		RHI::ShaderCompileDesc vertexCompile = {};
		vertexCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/PlaneDraw.vert.hlsl";
		vertexCompile.entryPoint = "VSMain";
		vertexCompile.stage = RHI::ShaderStage::Vertex;
		List<u8> vertexCode = RHI::ShaderCompiler::Compile(vertexCompile);

		RHI::ShaderCompileDesc fragmentCompile = {};
		fragmentCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/SkyCompositePass.frag.hlsl";
		fragmentCompile.entryPoint = "PSMain";
		fragmentCompile.stage = RHI::ShaderStage::Pixel;
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
			Terminal::Error(StringHelper::GetTypeName(this), "SkyCompositePass shaders could not be created");
			return false;
		}

		RHI::ColorTarget colorTarget = {};
		colorTarget.format = Engine::PresentationService::SceneFormat;

		RHI::GraphicsPipelineDesc pipelineDesc = {};
		pipelineDesc.pLayout = pContext->GetGlobalLayout();
		pipelineDesc.pVertexShader = m_vertexShader.Get();
		pipelineDesc.pPixelShader = m_fragmentShader.Get();
		pipelineDesc.rasterizer.cullMode = RHI::CullMode::None;
		pipelineDesc.depthStencil.depthTest = true;
		pipelineDesc.depthStencil.depthWrite = false;
		pipelineDesc.depthStencil.depthCompare = RHI::CompareOp::Equal;
		pipelineDesc.colorTargets.PushBack(colorTarget);
		pipelineDesc.depthFormat = RHI::TextureFormat::D32_FLOAT;
		pipelineDesc.sampleCount = RHI::SampleCount::Count8;

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "SkyCompositePass pipeline could not be created");
			return false;
		}

		m_resourceHeap = pContext->GetResourceHeap();
		m_skyView = m_resourceHeap->Allocate();

		return true;
	}

	void SkyCompositePass::OnFinalize()
	{
		if (m_skyView != kInvalid32)
		{
			m_resourceHeap->Free(m_skyView);
			m_skyView = kInvalid32;
		}

		m_pipeline.Reset();
		m_fragmentShader.Reset();
		m_vertexShader.Reset();
	}

	void SkyCompositePass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
		graph.Reads<Resource::SkyOutput>(RHI::ResourceState::ShaderResource);
		graph.Reads<Resource::PreDepthOutput>(RHI::ResourceState::DepthRead);
		graph.Writes<Resource::SceneColorMS>(RHI::ResourceState::RenderTarget);

		graph.RunsAfter<ForwardPass, ProceduralSkyPass>();
		graph.RunsBefore<ResolvePass>();
	}

	void SkyCompositePass::Render(const Engine::Blackboard& board)
	{
		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pColor = GetTexture<Resource::SceneColorMS>();

		m_resourceHeap->Write(m_skyView, RHI::DescriptorType::Texture, GetTextureView<Resource::SkyOutput>());

		RHI::RenderingDesc desc = {};
		desc.colorAttachments[0].pView = GetTextureView<Resource::SceneColorMS>();
		desc.colorAttachments[0].loadOp = RHI::LoadOp::Load;
		desc.colorAttachments[0].storeOp = RHI::StoreOp::Store;
		desc.colorAttachmentCount = 1;

		desc.depthAttachment.pView = GetTextureView<Resource::PreDepthOutput>();
		desc.depthAttachment.loadOp = RHI::LoadOp::Load;
		desc.depthAttachment.storeOp = RHI::StoreOp::Store;
		desc.depthAttachment.readOnly = true;

		pCmdList->StartRendering(desc);

		RHI::Viewport viewport = {};
		viewport.width = static_cast<f32>(pColor->GetWidth());
		viewport.height = static_cast<f32>(pColor->GetHeight());
		pCmdList->SetViewport(viewport);

		RHI::Scissor scissor = {};
		scissor.width = static_cast<i32>(pColor->GetWidth());
		scissor.height = static_cast<i32>(pColor->GetHeight());
		pCmdList->SetScissor(scissor);

		pCmdList->BindPipeline(m_pipeline.Get());

		SkyCompositeConstants constants = {};
		constants.skyTextureIndex = m_skyView;

		pCmdList->SetPushConstants(&constants, sizeof(SkyCompositeConstants));
		pCmdList->Draw(3);

		pCmdList->EndRendering();
	}
}