#include "DepthPrePass.h"

#include <Engine/Asset/Mesh/MeshStreamer.h>
#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Presentation/PresentationService.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>

#include <Engine/Rendering/Renderables/RenderableArea.h>
#include <Engine/Rendering/Renderables/RenderableCamera.h>
#include <Engine/Rendering/Renderables/RenderableDirectionalLight.h>
#include <Engine/Rendering/Renderables/RenderableMesh.h>
#include <Engine/Rendering/Renderables/RenderablePointLight.h>

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

	b8 DepthPrePass::OnInitialize()
	{
		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, DepthPrePass cannot be created");
			return false;
		}

		RHI::Device* pDevice = pContext->GetDevice();

		RHI::ShaderCompileDesc vertexCompile = {};
		vertexCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/DepthPrePass.vert.hlsl";
		vertexCompile.entryPoint = "VSMain";
		vertexCompile.stage = RHI::ShaderStage::Vertex;
		vertexCompile.target = RHI::ShaderTarget(pDevice->GetBackend());
		List<u8> vertexCode = RHI::ShaderCompiler::Compile(vertexCompile);

		RHI::ShaderModuleDesc vertexModule = {};
		vertexModule.stage = RHI::ShaderStage::Vertex;
		vertexModule.entryPoint = "VSMain";
		vertexModule.byteCode = vertexCode;
		m_vertexShader = pDevice->CreateRHIShaderModule(vertexModule);

		if (!m_vertexShader)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "DepthPrePass shaders could not be created");
			return false;
		}

		RHI::GraphicsPipelineDesc pipelineDesc = {};
		pipelineDesc.pLayout = pContext->GetGlobalLayout();
		pipelineDesc.pVertexShader = m_vertexShader.Get();
		pipelineDesc.pPixelShader = nullptr;
		pipelineDesc.rasterizer.cullMode = RHI::CullMode::None;
		pipelineDesc.depthStencil.depthTest = true;
		pipelineDesc.depthStencil.depthWrite = true;
		pipelineDesc.depthStencil.depthCompare = RHI::CompareOp::Greater;
		pipelineDesc.depthFormat = RHI::TextureFormat::D32_FLOAT;
		pipelineDesc.sampleCount = RHI::SampleCount::Count8;

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "DepthPrePass pipeline could not be created");
			return false;
		}

		return true;
	}

	void DepthPrePass::OnFinalize()
	{
		m_pipeline.Reset();
		m_vertexShader.Reset();
	}

	void DepthPrePass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
		auto* pArea = blackboard.Get<Engine::RenderableArea>();
		if (!pArea)
			return;

		RHI::TextureDesc depthDesc = {};
		depthDesc.format = RHI::TextureFormat::D32_FLOAT;
		depthDesc.usage = RHI::TextureUsage::DepthStencil | RHI::TextureUsage::Sampled;
		depthDesc.width = pArea->colorImageSize.X();
		depthDesc.height = pArea->colorImageSize.Y();
		depthDesc.sampleCount = RHI::SampleCount::Count8;

		graph.Creates<Resource::PreDepthOutput>(depthDesc);
		graph.Writes<Resource::PreDepthOutput>(RHI::ResourceState::DepthWrite);
	}

	void DepthPrePass::Render(const Engine::Blackboard& board)
	{
		const Engine::RenderableCamera* pCamera = board.Get<Engine::RenderableCamera>();
		const Engine::FrameMesh* pMeshes = board.Get<Engine::FrameMesh>();
		if (!pCamera || !pMeshes)
			return;

		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pDepth = GetTexture<Resource::PreDepthOutput>();

		RHI::RenderingDesc desc = {};
		desc.colorAttachmentCount = 0;

		desc.depthAttachment.pView = GetTextureView<Resource::PreDepthOutput>();
		desc.depthAttachment.loadOp = RHI::LoadOp::Clear;
		desc.depthAttachment.storeOp = RHI::StoreOp::Store;
		desc.depthAttachment.clearDepth = 0.0f;
		desc.depthAttachment.readOnly = false;

		pCmdList->StartRendering(desc);

		RHI::Viewport viewport = {};
		viewport.width = static_cast<f32>(pDepth->GetWidth());
		viewport.height = static_cast<f32>(pDepth->GetHeight());
		pCmdList->SetViewport(viewport);

		RHI::Scissor scissor = {};
		scissor.width = static_cast<i32>(pDepth->GetWidth());
		scissor.height = static_cast<i32>(pDepth->GetHeight());
		pCmdList->SetScissor(scissor);

		pCmdList->BindPipeline(m_pipeline.Get());

		for (const Engine::RenderableMesh& renderable : pMeshes->meshes)
		{
			if (!renderable.visible)
				continue;

			Asset::MeshStreamer* pStreamer = renderable.mesh.pAsset->GetLoader<Asset::MeshStreamer>();
			pCmdList->BindIndexBuffer(pStreamer->GetIndexBuffer());

			ForwardConstants constants = {};
			constants.worldViewProj = pCamera->camViewProjMatrix * renderable.worldMatrix;
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