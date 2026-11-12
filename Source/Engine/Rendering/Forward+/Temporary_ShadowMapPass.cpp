#include "Temporary_ShadowMapPass.h"

#include <Engine/Asset/Mesh/MeshStreamer.h>
#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>

#include <Engine/Rendering/Renderables/RenderableArea.h>
#include <Engine/Rendering/Renderables/RenderableMesh.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/RHI/Command/RenderingDesc.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Pipeline/GraphicsPipelineDesc.h>
#include <Runtime/RHI/Shader/ShaderCompiler.h>
#include <Runtime/RHI/Shader/ShaderModuleDesc.h>
#include <Runtime/RHI/Texture/Texture.h>

#include <cmath>

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

	Math::Vec3f ShadowMapPass::ResolveLightDirection(const Engine::RenderableDirectionalLight* pLight)
	{
		Math::Vec3f direction = { -0.3f, -0.8f, -0.5f };

		if (pLight && (pLight->direction | pLight->direction) > 1e-6f)
			direction = pLight->direction;

		return direction / std::sqrt(direction | direction);
	}

	Math::Mat4f ShadowMapPass::BuildLightViewProj(const Engine::RenderableCamera& camera, const Math::Vec3f& lightDirection, u32 mapSize)
	{
		const Math::Vec3f up = std::abs(lightDirection.Y()) > 0.99f ? Math::Vec3f(0.f, 0.f, 1.f) : Math::Vec3f(0.f, 1.f, 0.f);
		const Math::Mat4f view = Math::Mat4f::LookAt(Math::Vec3f::Zero(), lightDirection, up);
		const Math::Vec3f center = view.TransformPoint(camera.camWorldPosition);

		const f32 texel = (ShadowHalfExtent * 2.0f) / static_cast<f32>(mapSize);
		const f32 centerX = std::floor(center.X() / texel) * texel;
		const f32 centerY = std::floor(center.Y() / texel) * texel;
		const f32 distance = -center.Z();

		const Math::Mat4f proj = Math::Mat4f::Orthographic(centerX - ShadowHalfExtent, centerX + ShadowHalfExtent, centerY - ShadowHalfExtent, centerY + ShadowHalfExtent, distance + ShadowHalfDepth, distance - ShadowHalfDepth);

		return proj * view;
	}

	b8 ShadowMapPass::OnInitialize()
	{
		Engine::GraphicsContext* pContext = GetEngine()->RequestContext<Engine::GraphicsContext>();
		if (pContext == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, ShadowMapPass cannot be created");
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
			Terminal::Error(StringHelper::GetTypeName(this), "ShadowMapPass shaders could not be created");
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
		pipelineDesc.sampleCount = RHI::SampleCount::Count1;

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ShadowMapPass pipeline could not be created");
			return false;
		}

		return true;
	}

	void ShadowMapPass::OnFinalize()
	{
		m_pipeline.Reset();
		m_vertexShader.Reset();
	}

	void ShadowMapPass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
		auto* pArea = blackboard.Get<Engine::RenderableArea>();
		if (!pArea)
			return;

		RHI::TextureDesc depthDesc = {};
		depthDesc.format = RHI::TextureFormat::D32_FLOAT;
		depthDesc.usage = RHI::TextureUsage::DepthStencil | RHI::TextureUsage::Sampled;
		depthDesc.width = pArea->shadowMapSize.X();
		depthDesc.height = pArea->shadowMapSize.Y();
		depthDesc.sampleCount = RHI::SampleCount::Count1;

		graph.Creates<Resource::ShadowDepth>(depthDesc);
		graph.Writes<Resource::ShadowDepth>(RHI::ResourceState::DepthWrite);
	}

	void ShadowMapPass::Render(const Engine::Blackboard& board)
	{
		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pDepth = GetTexture<Resource::ShadowDepth>();

		RHI::RenderingDesc desc = {};
		desc.colorAttachmentCount = 0;

		desc.depthAttachment.pView = GetTextureView<Resource::ShadowDepth>();
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

		const Engine::RenderableCamera* pCamera = board.Get<Engine::RenderableCamera>();
		const Engine::FrameMesh* pMeshes = board.Get<Engine::FrameMesh>();
		if (!pCamera || !pMeshes)
		{
			pCmdList->EndRendering();
			return;
		}

		const Math::Vec3f lightDirection = ResolveLightDirection(board.TryGet<Engine::RenderableDirectionalLight>());
		const Math::Mat4f lightViewProj = BuildLightViewProj(*pCamera, lightDirection, pDepth->GetWidth());

		pCmdList->BindPipeline(m_pipeline.Get());

		for (const Engine::RenderableMesh& renderable : pMeshes->meshes)
		{
			if (!renderable.castShadow)
				continue;

			Asset::MeshStreamer* pStreamer = renderable.mesh.pAsset->GetLoader<Asset::MeshStreamer>();
			pCmdList->BindIndexBuffer(pStreamer->GetIndexBuffer());

			ForwardConstants constants = {};
			constants.worldViewProj = lightViewProj * renderable.worldMatrix;
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
