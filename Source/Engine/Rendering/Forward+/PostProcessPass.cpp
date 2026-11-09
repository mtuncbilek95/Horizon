#include "PostProcessPass.h"

#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Presentation/PresentationService.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
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
		struct PostProcessConstants
		{
			u32 textureIndex;
			f32 exposure;
		};
	}

	b8 PostProcessPass::OnInitialize()
	{
		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, PostProcessPass cannot be created");
			return false;
		}

		RHI::Device* pDevice = pContext->GetDevice();
		m_resourceHeap = pContext->GetResourceHeap();

		RHI::ShaderCompileDesc vertexCompile = {};
		vertexCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/PostProcessPass.vert.hlsl";
		vertexCompile.entryPoint = "VSMain";
		vertexCompile.stage = RHI::ShaderStage::Vertex;
		List<u8> vertexCode = RHI::ShaderCompiler::Compile(vertexCompile);

		RHI::ShaderCompileDesc fragmentCompile = {};
		fragmentCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/PostProcessPass.frag.hlsl";
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
			Terminal::Error(StringHelper::GetTypeName(this), "PostProcessPass shaders could not be created");
			return false;
		}

		RHI::ColorTarget colorTarget = {};
		colorTarget.format = Engine::PresentationService::SceneFormat;

		RHI::GraphicsPipelineDesc pipelineDesc = {};
		pipelineDesc.pLayout = pContext->GetGlobalLayout();
		pipelineDesc.pVertexShader = m_vertexShader.Get();
		pipelineDesc.pPixelShader = m_fragmentShader.Get();
		pipelineDesc.rasterizer.cullMode = RHI::CullMode::None;
		pipelineDesc.colorTargets.PushBack(colorTarget);

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "PostProcessPass pipeline could not be created");
			return false;
		}

		m_inputView = m_resourceHeap->Allocate();

		return true;
	}

	void PostProcessPass::OnFinalize()
	{
		if (m_inputView != kInvalid32)
		{
			m_resourceHeap->Free(m_inputView);
			m_inputView = kInvalid32;
		}

		m_pipeline.Reset();
		m_fragmentShader.Reset();
		m_vertexShader.Reset();

		m_resourceHeap = nullptr;
	}

	void PostProcessPass::Setup(const Engine::RenderSetupContext& setupContext, Engine::RenderDependency& graph)
	{
		RHI::TextureDesc outputDesc = {};
		outputDesc.format = Engine::PresentationService::SceneFormat;
		outputDesc.usage = RHI::TextureUsage::RenderTarget | RHI::TextureUsage::Sampled;
		outputDesc.width = setupContext.colorImageDimension.X();
		outputDesc.height = setupContext.colorImageDimension.Y();

		graph.Creates<Resource::PostProcessOutput>(outputDesc);
		graph.Reads<Resource::HDRColor>(RHI::ResourceState::ShaderResource);
		graph.Writes<Resource::PostProcessOutput>(RHI::ResourceState::RenderTarget);
	}

	void PostProcessPass::Render(const Engine::RenderDrawContext& context)
	{
		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pTarget = GetTexture<Resource::PostProcessOutput>();

		m_resourceHeap->Write(m_inputView, RHI::DescriptorType::Texture, GetTextureView<Resource::HDRColor>());

		RHI::RenderingDesc desc = {};
		desc.colorAttachments[0].pView = GetTextureView<Resource::PostProcessOutput>();
		desc.colorAttachments[0].loadOp = RHI::LoadOp::Clear;
		desc.colorAttachments[0].storeOp = RHI::StoreOp::Store;
		desc.colorAttachmentCount = 1;

		pCmdList->StartRendering(desc);

		RHI::Viewport viewport = {};
		viewport.width = static_cast<f32>(pTarget->GetWidth());
		viewport.height = static_cast<f32>(pTarget->GetHeight());
		pCmdList->SetViewport(viewport);

		RHI::Scissor scissor = {};
		scissor.width = static_cast<i32>(pTarget->GetWidth());
		scissor.height = static_cast<i32>(pTarget->GetHeight());
		pCmdList->SetScissor(scissor);

		PostProcessConstants constants = {};
		constants.textureIndex = m_inputView;
		constants.exposure = m_exposure;

		pCmdList->BindPipeline(m_pipeline.Get());
		pCmdList->SetPushConstants(&constants, sizeof(PostProcessConstants));
		pCmdList->Draw(3);

		pCmdList->EndRendering();
	}
}