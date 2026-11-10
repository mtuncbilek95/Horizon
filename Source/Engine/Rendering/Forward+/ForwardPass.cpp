#include "ForwardPass.h"

#include <Engine/Asset/Mesh/MeshStreamer.h>
#include <Engine/Core/Engine.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Presentation/PresentationService.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Engine/Rendering/Forward+/Resources.h>

#include <Engine/Rendering/RenderableCamera.h>
#include <Engine/Rendering/RenderableDirectionalLight.h>
#include <Engine/Rendering/RenderableMesh.h>
#include <Engine/Rendering/RenderablePointLight.h>

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
		constexpr u32 kMaxPointLights = 4096;

		struct ForwardConstants
		{
			Math::Mat4f worldViewProj;
			f32 worldRows[12];
			u32 vertexBufferIndex;
			u32 firstVertex;
			u32 pointLightBufferIndex;
			u32 lightFrameBufferIndex;
		};

		struct GpuPointLight
		{
			f32 colorIntensity[4];
			f32 positionRange[4];
		};

		struct GpuLightFrame
		{
			u32 pointLightCount;
			u32 padding[3];
			f32 directionalColorIntensity[4];
			f32 directionalDirection[4];
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
		pipelineDesc.sampleCount = RHI::SampleCount::Count8;

		m_pipeline = pDevice->CreateRHIPipeline(pipelineDesc);
		if (!m_pipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ForwardPass pipeline could not be created");
			return false;
		}

		RHI::BufferDesc pointLightDesc = {};
		pointLightDesc.size = sizeof(GpuPointLight) * kMaxPointLights;
		pointLightDesc.usage = RHI::BufferUsage::Storage;
		pointLightDesc.memory = RHI::MemoryType::Upload;
		m_pointLightBuffer = pDevice->CreateRHIBuffer(pointLightDesc);

		if (!m_pointLightBuffer || m_pointLightBuffer->GetMappedData() == nullptr)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ForwardPass point light buffer could not be created");
			return false;
		}

		m_pointLightBuffer->SetDebugName("ForwardPass_PointLightBuffer");

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
		m_resourceHeap->Write(m_pointLightView, RHI::DescriptorType::Buffer, m_pointLightBuffer.Get(), 0, pointLightDesc.size, sizeof(GpuPointLight));

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

		if (m_pointLightView != kInvalid32)
		{
			m_resourceHeap->Free(m_pointLightView);
			m_pointLightView = kInvalid32;
		}

		m_lightFrameBuffer.Reset();
		m_pointLightBuffer.Reset();
		m_pipeline.Reset();
		m_fragmentShader.Reset();
		m_vertexShader.Reset();
	}

	void ForwardPass::Setup(const Engine::RenderSetupContext& setupContext, Engine::RenderDependency& graph)
	{
		RHI::TextureDesc colorDesc = {};
		colorDesc.format = Engine::PresentationService::SceneFormat;
		colorDesc.usage = RHI::TextureUsage::RenderTarget;
		colorDesc.width = setupContext.colorImageDimension.X();
		colorDesc.height = setupContext.colorImageDimension.Y();
		colorDesc.sampleCount = RHI::SampleCount::Count8;

		RHI::TextureDesc depthDesc = {};
		depthDesc.format = RHI::TextureFormat::D32_FLOAT;
		depthDesc.usage = RHI::TextureUsage::DepthStencil | RHI::TextureUsage::Sampled;
		depthDesc.width = setupContext.colorImageDimension.X();
		depthDesc.height = setupContext.colorImageDimension.Y();
		depthDesc.sampleCount = RHI::SampleCount::Count8;

		graph.Creates<Resource::SceneColorMS>(colorDesc);
		graph.Creates<Resource::PreDepthOutput>(depthDesc);
		graph.Writes<Resource::PreDepthOutput>(RHI::ResourceState::DepthWrite);
		graph.Writes<Resource::SceneColorMS>(RHI::ResourceState::RenderTarget);
	}

	void ForwardPass::Render(const Engine::Blackboard& board)
	{
		const Engine::RenderableCamera* pCamera = board.Get<Engine::RenderableCamera>();
		const Engine::FrameMesh* pMeshes = board.Get<Engine::FrameMesh>();
		if (!pCamera || !pMeshes)
			return;

		const Engine::FramePointLight* pLights = board.TryGet<Engine::FramePointLight>();
		const Engine::RenderableDirectionalLight* pDirectional = board.TryGet<Engine::RenderableDirectionalLight>();

		RHI::CommandList* pCmdList = GetCommandList();
		RHI::Texture* pColor = GetTexture<Resource::SceneColorMS>();

		GpuLightFrame lightFrame = {};

		if (pLights)
		{
			lightFrame.pointLightCount = static_cast<u32>(std::min<usize>(pLights->pointLights.GetCount(), kMaxPointLights));
			GpuPointLight* pPointLights = reinterpret_cast<GpuPointLight*>(m_pointLightBuffer->GetMappedData());

			for (u32 i = 0; i < lightFrame.pointLightCount; i++)
			{
				const Engine::RenderablePointLight& light = pLights->pointLights[i];

				pPointLights[i] =
				{
					{ light.color.R(), light.color.G(), light.color.B(), light.intensity },
					{ light.position.X(), light.position.Y(), light.position.Z(), light.range },
				};
			}
		}

		if (pDirectional)
		{
			lightFrame.directionalColorIntensity[0] = pDirectional->color.R();
			lightFrame.directionalColorIntensity[1] = pDirectional->color.G();
			lightFrame.directionalColorIntensity[2] = pDirectional->color.B();
			lightFrame.directionalColorIntensity[3] = pDirectional->intensity;

			lightFrame.directionalDirection[0] = pDirectional->direction.X();
			lightFrame.directionalDirection[1] = pDirectional->direction.Y();
			lightFrame.directionalDirection[2] = pDirectional->direction.Z();
		}

		*reinterpret_cast<GpuLightFrame*>(m_lightFrameBuffer->GetMappedData()) = lightFrame;

		RHI::RenderingDesc desc = {};
		desc.colorAttachments[0].pView = GetTextureView<Resource::SceneColorMS>();
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

			for (i32 row = 0; row < 3; row++)
			{
				for (i32 col = 0; col < 4; col++)
					constants.worldRows[row * 4 + col] = renderable.worldMatrix(row, col);
			}

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