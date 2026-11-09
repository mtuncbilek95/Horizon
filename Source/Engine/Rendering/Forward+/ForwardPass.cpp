#include "ForwardPass.h"

#include <Engine/Asset/Mesh/MeshStreamer.h>
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
		struct ForwardConstants
		{
			Math::Mat4f worldViewProj;
			u32 vertexBufferIndex;
			u32 firstVertex;
		};
	}

	b8 ForwardPass::OnInitialize()
	{
		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, ForwardPass cannot be created");
			return false;
		}

		RHI::Device* pDevice = pContext->GetDevice();

		RHI::ShaderCompileDesc vertexCompile = {};
		vertexCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/ForwardPass.vert.hlsl";
		vertexCompile.entryPoint = "VSMain";
		vertexCompile.stage = RHI::ShaderStage::Vertex;
		List<u8> vertexCode = RHI::ShaderCompiler::Compile(vertexCompile);

		RHI::ShaderCompileDesc fragmentCompile = {};
		fragmentCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/ForwardPass.frag.hlsl";
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
			Terminal::Error(StringHelper::GetTypeName(this), "ForwardPass shaders could not be created");
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
		pipelineDesc.depthStencil.depthWrite = true;
		pipelineDesc.depthStencil.depthCompare = RHI::CompareOp::Greater;
		pipelineDesc.colorTargets.PushBack(colorTarget);
		pipelineDesc.depthFormat = RHI::TextureFormat::D32_FLOAT;

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ForwardPass pipeline could not be created");
			return false;
		}

		return true;
	}

	void ForwardPass::OnFinalize()
	{
		m_pipeline.Reset();
		m_fragmentShader.Reset();
		m_vertexShader.Reset();
	}

	void ForwardPass::Setup(const Engine::RenderSetupContext& setupContext, Engine::RenderDependency& graph)
	{
		RHI::TextureDesc depthDesc = {};
		depthDesc.format = RHI::TextureFormat::D32_FLOAT;
		depthDesc.usage = RHI::TextureUsage::DepthStencil | RHI::TextureUsage::Sampled;
		depthDesc.width = setupContext.colorImageDimension.X();
		depthDesc.height = setupContext.colorImageDimension.Y();

		graph.Creates<Resource::PreDepthOutput>(depthDesc);
		graph.Writes<Resource::PreDepthOutput>(RHI::ResourceState::DepthWrite);
		graph.Writes<Resource::SceneColor>(RHI::ResourceState::RenderTarget);
	}

	void ForwardPass::Render(const Engine::RenderDrawContext& context)
	{
		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pColor = GetTexture<Resource::SceneColor>();

		RHI::RenderingDesc desc = {};
		desc.colorAttachments[0].pView = GetTextureView<Resource::SceneColor>();
		desc.colorAttachments[0].loadOp = RHI::LoadOp::Clear;
		desc.colorAttachments[0].storeOp = RHI::StoreOp::Store;
		desc.colorAttachments[0].clearColor = { 0.0f, 0.0f, 0.0f, 1.f };
		desc.colorAttachmentCount = 1;

		desc.depthAttachment.pView = GetTextureView<Resource::PreDepthOutput>();
		desc.depthAttachment.loadOp = RHI::LoadOp::Clear;
		desc.depthAttachment.storeOp = RHI::StoreOp::Store;
		desc.depthAttachment.clearDepth = 0.0f;
		desc.depthAttachment.readOnly = false;

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

		for (const Engine::RenderableMesh& renderable : context.meshes)
		{
			if (!renderable.visible)
				continue;

			Asset::MeshStreamer* pStreamer = renderable.mesh.pAsset->GetLoader<Asset::MeshStreamer>();
			pCmdList->BindIndexBuffer(pStreamer->GetIndexBuffer());

			ForwardConstants constants = {};
			constants.worldViewProj = context.camViewProjMatrix * renderable.worldMatrix;
			constants.vertexBufferIndex = pStreamer->GetVertexBufferView();

			for (const Asset::MeshSubmesh& submesh : renderable.mesh.submeshes)
			{
				constants.firstVertex = renderable.mesh.firstVertex + submesh.vertexOffset;

				pCmdList->SetPushConstants(&constants, sizeof(ForwardConstants));
				pCmdList->DrawIndexed(submesh.indexCount, 1, renderable.mesh.firstIndex + submesh.indexOffset);
			}
		}

		pCmdList->EndRendering();
	}
}