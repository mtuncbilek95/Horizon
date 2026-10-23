#include "RenderSystem.h"

#include <Engine/World/Components/Physics/TransformComponent.h>
#include <Engine/World/Components/Rendering/CameraComponent.h>
#include <Engine/World/Components/Rendering/MeshComponent.h>
#include <Engine/World/Components/PostProcess/FogComponent.h>
#include <Engine/World/Systems/Rendering/CameraSystem.h>
#include <Engine/Rendering/RenderGraph/RenderPass.h>
#include <Engine/Rendering/RenderGraph/RenderFrameData.h>
#include <Engine/Rendering/RenderGraph/RenderOrderAttribute.h>
#include <Engine/Asset/AssetService.h>
#include <Engine/Asset/Mesh/MeshAssetStreamer.h>
#include <Engine/Asset/Mesh/MeshVertex.h>

#include <Runtime/Log/Terminal.h>
#include <Runtime/RHI/Buffer/GfxBuffer.h>
#include <Runtime/RHI/Shader/GfxShaderCompiler.h>
#include <Runtime/RHI/Shader/GfxShader.h>
#include <Runtime/RHI/Shader/GfxShaderDesc.h>
#include <Runtime/RHI/Pipeline/GfxGraphicsPipelineDesc.h>
#include <Runtime/RHI/Pipeline/GfxPipeline.h>

#include <Runtime/Math/Mat4f.h>

namespace Horizon::Engine
{
	struct PassEntry
	{
		RenderPass* pPass = nullptr;
		i32 order = 0;
	};

	b8 RenderSystem::OnInitialize()
	{
		m_context = GetEngine()->RequestContext<GraphicsContext>();

		if (!m_context)
		{
			Terminal::Error(StringOps::GetName(this), "GraphicsContext is unavailable, render system stays down");
			return false;
		}

		m_device = m_context->GetDevice();
		m_queue = m_context->GetGraphicsQueue();

		m_colorTargetHeap = m_context->GetColorTargetHeap();
		m_depthTargetHeap = m_context->GetDepthTargetHeap();
		m_resourceHeap = m_context->GetResourceHeap();

		m_frameDataStride = (sizeof(RenderFrameData) + 255) & ~255u;

		RHI::GfxBufferDesc frameDataDesc = {};
		frameDataDesc.usage = RHI::GfxBufferUsage::Constant;
		frameDataDesc.memory = RHI::GfxMemoryType::GpuUpload;
		frameDataDesc.size = usize(m_frameDataStride) * GraphicsContext::MaxFramesInFlight;

		m_frameDataBuffer = m_device->CreateBuffer(frameDataDesc);
		if (!m_frameDataBuffer)
		{
			Terminal::Error(StringOps::GetName(this), "FrameData buffer could not be created, render system stays down");
			return false;
		}

		m_frameDataBuffer->SetDebugName("RenderSystem_FrameData");
		m_frameDataMap = (u8*)m_frameDataBuffer->Map();

		for (u32 i = 0; i < GraphicsContext::MaxFramesInFlight; i++)
			m_frameDataViews[i] = m_resourceHeap->CreateConstantView(m_frameDataBuffer, usize(i) * m_frameDataStride, m_frameDataStride);

		ResizeImage({ 1280, 720 });
		m_slots.Resize(GraphicsContext::MaxFramesInFlight);
		m_fence = m_device->CreateFence();

		auto* pReflection = GetEngine()->GetReflectionSystem();

		// TODO: This is not a correct solution. 
		List<Reflect::Type*> types = pReflection->GetTypeByBase(Reflect::TypeOf<RenderPass>());

		List<PassEntry> entries;
		for (auto* pType : types)
		{
			auto* pAttr = pType->GetCustomAttribute<RenderOrderAttribute>();
			if (!pAttr)
				continue;

			RenderPass* pPass = (RenderPass*)pType->Create();
			if (!pPass)
			{
				Terminal::Warn(StringOps::GetName(this), "{} may have virtual function issues.", pType->GetName());
				continue;
			}
			pPass->m_engine = GetEngine();

			PassEntry entry = {};
			entry.pPass = pPass;
			entry.order = pAttr->GetOrder();
			entries.PushBack(entry);
		}

		entries.Sort([](const PassEntry& left, const PassEntry& right)
			{
				return left.order < right.order;
			});

		for (const PassEntry& entry : entries)
		{
			entry.pPass->OnInitialize();
			m_renderPasses.PushBack(entry.pPass);
		}

		return true;
	}

	void RenderSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		CameraSystem* pCameraSystem = RequestSystem<CameraSystem>();
		CameraComponent* pCamera = nullptr;

		// Find the correct camera
		if (pCameraSystem != nullptr && pCameraSystem->GetActiveCamera().IsValid())
			pCamera = currentScene.FindComponent<CameraComponent>(pCameraSystem->GetActiveCamera());

		if (pCamera != nullptr && pCamera->m_targetScreen.X() >= 1.0f && pCamera->m_targetScreen.Y() >= 1.0f)
			ResizeImage({ u32(pCamera->m_targetScreen.X()), u32(pCamera->m_targetScreen.Y()) });

		if (!pCamera)
		{
			Terminal::Error(StringOps::GetName(this), "There is no valid camera somehow");
			return;
		}

		RenderSlot& slot = m_slots[m_frameIndex];

		if (slot.currSize != m_targetSize)
		{
			DestroySlot(m_frameIndex);
			CreateSlot(m_frameIndex);

			if (!slot.pTargetTexture)
				return;
		}

		AssetService* pAssetService = GetEngine()->RequestService<AssetService>();
		MeshAssetStreamer* pMeshStreamer = pAssetService->FindLoader<MeshAssetStreamer>(Reflect::TypeOf<MeshAsset>());

		RenderPassFrame currentFrame = {};
		currentFrame.cameraPosition = pCamera->m_worldPosition;
		currentFrame.viewMatrix = pCamera->m_view;
		currentFrame.projMatrix = pCamera->m_projection;
		currentFrame.projMatrix = pCamera->m_viewProjection;
		currentFrame.targetSize = m_targetSize;
		currentFrame.frameIndex = m_frameIndex;
		currentFrame.pEngineFrame = &const_cast<EngineFrame&>(ctx);
		currentFrame.pColorTarget = slot.pTargetTexture;
		currentFrame.pCmdList = slot.pTargetCmd;
		currentFrame.pDepthTarget = slot.pTargetDepth;
		currentFrame.pIndexBuffer = pMeshStreamer->GetIndexBuffer();
		currentFrame.vertexBufferIndex = pMeshStreamer->GetVertexBufferView();

		// TODO: Gather all the renderable objects here?
		currentScene.ForEach<TransformComponent, MeshComponent>([&](EntityHandle entity, TransformComponent& transComp, MeshComponent& meshComp)
			{
				if (meshComp.m_hideInRender)
				{
					Terminal::Log(StringOps::GetName(this), "WILL HIDE STOP?");
					return;
				}

				MeshAsset* pMesh = meshComp.m_meshHandle.GetAsset();

				if (!pMesh)
				{
					Terminal::Log(StringOps::GetName(this), "WILL NULLPTR STOP?");
					return;
				}

				if (pMesh->GetResidency() != AssetResidency::Resident)
				{
					Terminal::Log(StringOps::GetName(this), "WILL LOAD STOP?");
					pMesh->LoadAsync();
					return;
				}

				for (const MeshAssetSubmesh& submesh : pMesh->GetSubmeshes())
				{
					RenderableObject object = {};
					object.firstVertex = pMesh->GetFirstVertex() + submesh.vertexOffset;
					object.firstIndex = pMesh->GetFirstIndex() + submesh.indexOffset;
					object.indexCount = submesh.indexCount;
					object.worldMatrix = transComp.m_worldMatrix;

					currentFrame.graphReadyObjects.PushBack(object);
					Terminal::Log(StringOps::GetName(this), "PUSHING THE MESH");
				}
			});

		m_fence->WaitCPU(slot.fenceValue);

		RenderFrameData frameData = {};
		frameData.view = pCamera->m_view;
		frameData.proj = pCamera->m_projection;
		frameData.viewProj = pCamera->m_viewProjection;
		frameData.cameraPosition = pCamera->m_worldPosition;
		frameData.time = f32(ctx.GetTotalSeconds());

		std::memcpy(m_frameDataMap + usize(m_frameIndex) * m_frameDataStride, &frameData, sizeof(RenderFrameData));
		currentFrame.frameDataIndex = m_frameDataViews[m_frameIndex];

		slot.pTargetCmd->Begin();
		slot.pTargetCmd->BindDescriptorHeaps(m_resourceHeap, nullptr);
		{
			RHI::GfxTextureBarrier beginBarrier = {};
			beginBarrier.pTexture = slot.pTargetTexture;
			beginBarrier.before = slot.currState;
			beginBarrier.after = slot.currState = RHI::GfxResourceState::RenderTarget;
			beginBarrier.firstMip = 0;
			beginBarrier.firstSlice = 0;
			beginBarrier.mipCount = 1;
			beginBarrier.sliceCount = 1;
			slot.pTargetCmd->Barrier(&beginBarrier, 1);
		}

		for (auto* pPass : m_renderPasses)
			pPass->RenderFrame(currentFrame);

		{
			RHI::GfxTextureBarrier endBarrier = {};
			endBarrier.pTexture = slot.pTargetTexture;
			endBarrier.before = slot.currState;
			endBarrier.after = slot.currState = RHI::GfxResourceState::ShaderResource;
			endBarrier.firstMip = 0;
			endBarrier.firstSlice = 0;
			endBarrier.mipCount = 1;
			endBarrier.sliceCount = 1;
			slot.pTargetCmd->Barrier(&endBarrier, 1);
		}

		slot.pTargetCmd->End();

		m_queue->Submit(&slot.pTargetCmd, 1);
		slot.fenceValue = m_queue->Signal(m_fence);

		m_frameIndex = (m_frameIndex + 1) % GraphicsContext::MaxFramesInFlight;
	}

	void RenderSystem::OnFinalize()
	{
		if (!m_device)
			return;

		m_device->WaitIdle();

		if (m_frameDataBuffer)
		{
			for (u32 i = 0; i < GraphicsContext::MaxFramesInFlight; i++)
			{
				if (m_frameDataViews[i] != kInvalid32)
					m_resourceHeap->Free(m_frameDataViews[i]);
			}

			m_frameDataBuffer->Unmap();
			Memory::Allocator::Delete(m_frameDataBuffer);
			m_frameDataBuffer = nullptr;
			m_frameDataMap = nullptr;
		}

		for (auto* pPass : m_renderPasses)
			pPass->OnFinalize();

		for (auto* pPass : m_renderPasses)
			Memory::Allocator::Delete(pPass);

		m_renderPasses.Clear();

		for (usize i = 0; i < m_slots.GetCount(); i++)
			DestroySlot(i);

		m_slots.Clear();

		Memory::Allocator::Delete(m_fence);
		m_fence = nullptr;

		m_resourceHeap = nullptr;
		m_depthTargetHeap = nullptr;
		m_colorTargetHeap = nullptr;
	}

	void RenderSystem::OnGroupsChanged(SystemGroup previous, SystemGroup current)
	{
	}

	void RenderSystem::ResizeImage(const Math::Vec2u& imgSize)
	{
		u32 width = Math::Max(1u, imgSize.X());
		u32 height = Math::Max(1u, imgSize.Y());

		if (width == m_targetSize.X() || height == m_targetSize.Y())
			return;

		m_targetSize = { width, height };
	}

	void RenderSystem::CreateSlot(usize index)
	{
		RenderSlot& slot = m_slots[index];

		slot.pTargetCmd = m_device->CreateCommandList(RHI::GfxQueueType::Graphics);

		RHI::GfxTextureDesc colorDesc = {};
		colorDesc.width = m_targetSize.X();
		colorDesc.height = m_targetSize.Y();
		colorDesc.type = RHI::GfxTextureType::Tex2D;
		colorDesc.format = RHI::GfxTextureFormat::RGBA8_UNORM;
		colorDesc.usage = RHI::GfxTextureUsage::RenderTarget | RHI::GfxTextureUsage::Sampled;
		colorDesc.clearColor = { 0.0f, 0.0f, 0.0f, 1.f };

		RHI::GfxTextureDesc depthDesc = {};
		depthDesc.width = m_targetSize.X();
		depthDesc.height = m_targetSize.Y();
		depthDesc.type = RHI::GfxTextureType::Tex2D;
		depthDesc.format = RHI::GfxTextureFormat::D32_FLOAT;
		depthDesc.usage = RHI::GfxTextureUsage::DepthStencil;

		slot.pTargetTexture = m_device->CreateTexture(colorDesc);
		slot.pTargetDepth = m_device->CreateTexture(depthDesc);

		if (!slot.pTargetCmd || !slot.pTargetTexture || !slot.pTargetDepth)
		{
			Terminal::Error(StringOps::GetName(this), "Slot {} could not be created with {}x{}", index, m_targetSize.X(), m_targetSize.Y());
			return;
		}

		slot.pTargetTexture->SetDebugName("Scene - ColorTarget");
		slot.pTargetDepth->SetDebugName("Scene - DepthTarget");

		slot.renderTargetView = m_colorTargetHeap->CreateRenderTargetView(slot.pTargetTexture);
		slot.depthView = m_depthTargetHeap->CreateDepthStencilView(slot.pTargetDepth);
		slot.shaderView = m_resourceHeap->CreateShaderView(slot.pTargetTexture);

		slot.currState = RHI::GfxResourceState::Common;
		slot.currSize = m_targetSize;
		slot.fenceValue = 0;
	}

	void RenderSystem::DestroySlot(usize index)
	{
		RenderSlot& slot = m_slots[index];

		m_device->WaitIdle();

		if (slot.renderTargetView != kInvalid32)
			m_colorTargetHeap->Free(slot.renderTargetView);

		if (slot.depthView != kInvalid32)
			m_depthTargetHeap->Free(slot.depthView);

		if (slot.shaderView != kInvalid32)
			m_resourceHeap->Free(slot.shaderView);

		Memory::Allocator::Delete(slot.pTargetTexture);
		Memory::Allocator::Delete(slot.pTargetDepth);
		Memory::Allocator::Delete(slot.pTargetCmd);

		slot = RenderSlot();
	}

	u64 RenderSystem::GetOutputView() const
	{
		if (!m_slots[m_frameIndex].pTargetTexture)
			return kInvalid64;

		return m_resourceHeap->GetGpuHandle(m_slots[m_frameIndex].shaderView);
	}
}