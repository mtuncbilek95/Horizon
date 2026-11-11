#include "ForwardPass.h"

#include <Engine/Asset/Mesh/MeshStreamer.h>
#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Presentation/PresentationService.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>
#include <Engine/Rendering/Forward+/ClusterInformations.h>

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
			Math::Mat4f worldMatrix;
			u32 vertexBufferIndex;
			u32 firstVertex;
			u32 pointLightBufferIndex;
			u32 lightFrameBufferIndex;
			u32 lightGridIndex;
			u32 tileSize;
			u32 clusterCountX;
			u32 clusterCountY;
			u32 clusterCountZ;
			u32 maxLightsPerCluster;
			f32 nearPlane;
			f32 farPlane;
		};

		struct GpuLightFrame
		{
			Math::Vec3f sunColor;
			f32 intensity;
			Math::Vec3f direction;
			u32 pointLightCount;
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
		vertexCompile.target = RHI::ShaderTarget(pDevice->GetBackend());
		List<u8> vertexCode = RHI::ShaderCompiler::Compile(vertexCompile);

		RHI::ShaderCompileDesc fragmentCompile = {};
		fragmentCompile.filePath = HORIZON_RESOURCE_DIR "/Shaders/Forward+/ForwardPass.frag.hlsl";
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
		pipelineDesc.depthStencil.depthWrite = false;
		pipelineDesc.depthStencil.depthCompare = RHI::CompareOp::GreaterEqual;
		pipelineDesc.colorTargets.PushBack(colorTarget);
		pipelineDesc.depthFormat = RHI::TextureFormat::D32_FLOAT;
		pipelineDesc.sampleCount = RHI::SampleCount::Count8;

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ForwardPass pipeline could not be created");
			return false;
		}

		RHI::BufferDesc lightFrameDesc = {};
		lightFrameDesc.size = sizeof(GpuLightFrame);
		lightFrameDesc.usage = RHI::BufferUsage::Storage;
		lightFrameDesc.memory = RHI::MemoryType::Upload;
		m_lightFrameBuffer = pDevice->CreateRHIBuffer(lightFrameDesc);

		if (!m_lightFrameBuffer || m_lightFrameBuffer->GetMappedData() == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ForwardPass light frame buffer could not be created");
			return false;
		}

		m_lightFrameBuffer->SetDebugName("ForwardPass_LightFrameBuffer");

		m_resourceHeap = pContext->GetResourceHeap();
		m_pointLightView = m_resourceHeap->Allocate();
		m_lightGridView = m_resourceHeap->Allocate();

		m_lightFrameView = m_resourceHeap->Allocate();
		m_resourceHeap->Write(m_lightFrameView, RHI::DescriptorType::Buffer, m_lightFrameBuffer.Get(), 0, lightFrameDesc.size, sizeof(GpuLightFrame));

		return true;
	}

	void ForwardPass::OnFinalize()
	{
		if (m_lightFrameView != kInvalid32)
		{
			m_resourceHeap->Free(m_lightFrameView);
			m_lightFrameView = kInvalid32;
		}

		if (m_lightGridView != kInvalid32)
		{
			m_resourceHeap->Free(m_lightGridView);
			m_lightGridView = kInvalid32;
		}

		if (m_pointLightView != kInvalid32)
		{
			m_resourceHeap->Free(m_pointLightView);
			m_pointLightView = kInvalid32;
		}

		m_lightFrameBuffer.Reset();
		m_pipeline.Reset();
		m_fragmentShader.Reset();
		m_vertexShader.Reset();
	}

	void ForwardPass::Setup(const Engine::Blackboard& blackboard, Engine::RenderDependency& graph)
	{
		auto* pArea = blackboard.Get<Engine::RenderableArea>();
		if (!pArea)
			return;

		RHI::TextureDesc colorDesc = {};
		colorDesc.format = Engine::PresentationService::SceneFormat;
		colorDesc.usage = RHI::TextureUsage::RenderTarget;
		colorDesc.width = pArea->colorImageSize.X();
		colorDesc.height = pArea->colorImageSize.Y();
		colorDesc.sampleCount = RHI::SampleCount::Count8;

		graph.Creates<Resource::SceneColorMS>(colorDesc);
		graph.Reads<Resource::PreDepthOutput>(RHI::ResourceState::DepthRead);
		graph.Reads<Resource::PointLightList>(RHI::ResourceState::ShaderResource);
		graph.Reads<Resource::ClusteredLightGrid>(RHI::ResourceState::ShaderResource);
		graph.Writes<Resource::SceneColorMS>(RHI::ResourceState::RenderTarget);
	}

	void ForwardPass::Render(const Engine::Blackboard& board)
	{
		const Engine::RenderableCamera* pCamera = board.Get<Engine::RenderableCamera>();
		const Engine::FrameMesh* pMeshes = board.Get<Engine::FrameMesh>();
		if (!pCamera || !pMeshes)
			return;

		const Engine::RenderableDirectionalLight* pDirectional = board.TryGet<Engine::RenderableDirectionalLight>();

		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pColor = GetTexture<Resource::SceneColorMS>();
		RHI::Buffer* pPointLights = GetBuffer<Resource::PointLightList>();
		RHI::Buffer* pLightGrid = GetBuffer<Resource::ClusteredLightGrid>();

		m_resourceHeap->Write(m_pointLightView, RHI::DescriptorType::Buffer, pPointLights, 0, pPointLights->GetSize(), sizeof(Engine::RenderablePointLight));
		m_resourceHeap->Write(m_lightGridView, RHI::DescriptorType::Buffer, pLightGrid, 0, pLightGrid->GetSize(), sizeof(u32));

		GpuLightFrame lightFrame = {};

		if (pDirectional)
		{
			lightFrame.sunColor = Math::Vec3f(pDirectional->color.R(), pDirectional->color.G(), pDirectional->color.B());
			lightFrame.intensity = pDirectional->intensity;
			lightFrame.direction = pDirectional->direction;
		}

		*reinterpret_cast<GpuLightFrame*>(m_lightFrameBuffer->GetMappedData()) = lightFrame;

		RHI::RenderingDesc desc = {};
		desc.colorAttachments[0].pView = GetTextureView<Resource::SceneColorMS>();
		desc.colorAttachments[0].loadOp = RHI::LoadOp::Clear;
		desc.colorAttachments[0].storeOp = RHI::StoreOp::Store;
		desc.colorAttachments[0].clearColor = { 0.0f, 0.0f, 0.0f, 1.f };
		desc.colorAttachmentCount = 1;

		desc.depthAttachment.pView = GetTextureView<Resource::PreDepthOutput>();
		desc.depthAttachment.loadOp = RHI::LoadOp::Load;
		desc.depthAttachment.storeOp = RHI::StoreOp::Store;
		desc.depthAttachment.clearDepth = 0.0f;
		desc.depthAttachment.readOnly = true;

		pCmdList->StartRendering(desc);

		RHI::Viewport viewport = {};
		viewport.width = f32(pColor->GetWidth());
		viewport.height = f32(pColor->GetHeight());
		pCmdList->SetViewport(viewport);

		RHI::Scissor scissor = {};
		scissor.width = i32(pColor->GetWidth());
		scissor.height = i32(pColor->GetHeight());
		pCmdList->SetScissor(scissor);

		pCmdList->BindPipeline(m_pipeline.Get());

		const u32 clusterCountX = (pColor->GetWidth() + kClusterTileSize - 1) / kClusterTileSize;
		const u32 clusterCountY = (pColor->GetHeight() + kClusterTileSize - 1) / kClusterTileSize;

		for (const Engine::RenderableMesh& renderable : pMeshes->meshes)
		{
			if (!renderable.visible)
				continue;

			Asset::MeshStreamer* pStreamer = renderable.mesh.pAsset->GetLoader<Asset::MeshStreamer>();
			pCmdList->BindIndexBuffer(pStreamer->GetIndexBuffer());

			ForwardConstants constants = {};
			constants.worldViewProj = pCamera->camViewProjMatrix * renderable.worldMatrix;
			constants.vertexBufferIndex = pStreamer->GetVertexBufferView();
			constants.pointLightBufferIndex = m_pointLightView;
			constants.lightFrameBufferIndex = m_lightFrameView;
			constants.lightGridIndex = m_lightGridView;
			constants.tileSize = kClusterTileSize;
			constants.clusterCountX = clusterCountX;
			constants.clusterCountY = clusterCountY;
			constants.clusterCountZ = kClusterCountZ;
			constants.maxLightsPerCluster = kMaxLightsPerCluster;
			constants.nearPlane = pCamera->nearPlane;
			constants.farPlane = pCamera->farPlane;
			constants.worldMatrix = renderable.worldMatrix;

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