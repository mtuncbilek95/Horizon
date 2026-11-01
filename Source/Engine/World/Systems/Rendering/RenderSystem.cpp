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
#include <Runtime/RHI/Buffer/Buffer.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Descriptor/DescriptorHeap.h>
#include <Runtime/RHI/Device/Device.h>

#include <Runtime/Math/Mat4f.h>

#include <cstring>

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
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext is unavailable, render system stays down");
			return false;
		}

		m_device = m_context->GetDevice();

		m_resourceHeap = m_context->GetResourceHeap();
		m_samplerHeap = m_context->GetSamplerHeap();

		m_frameDataStride = (sizeof(RenderFrameData) + 255) & ~255u;

		RHI::BufferDesc frameDataDesc = {};
		frameDataDesc.usage = RHI::BufferUsage::Constant;
		frameDataDesc.memory = RHI::MemoryType::GpuUpload;
		frameDataDesc.size = u64(m_frameDataStride) * GraphicsContext::MaxFramesInFlight;

		m_frameDataBuffer = m_device->CreateRHIBuffer(frameDataDesc);
		if (!m_frameDataBuffer)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "FrameData buffer could not be created, render system stays down");
			return false;
		}

		m_frameDataBuffer->SetDebugName("RenderSystem_FrameData");

		for (u32 i = 0; i < GraphicsContext::MaxFramesInFlight; i++)
		{
			m_frameDataViews[i] = m_resourceHeap->Allocate();
			if (m_frameDataViews[i] == kInvalid32)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "FrameData view {} could not be allocated, render system stays down", i);
				return false;
			}

			m_resourceHeap->Write(m_frameDataViews[i], RHI::DescriptorType::ConstantBuffer, m_frameDataBuffer.Get(), u64(i) * m_frameDataStride, m_frameDataStride);
		}

		ResizeImage({ 1280, 720 });
		m_slots.Resize(GraphicsContext::MaxFramesInFlight);
		m_fence = m_device->CreateRHIFence({});

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
				Terminal::Warn(StringHelper::GetTypeName(this), "{} may have virtual function issues.", pType->GetName());
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
			Terminal::Error(StringHelper::GetTypeName(this), "There is no valid camera somehow");
			return;
		}

		RenderSlot& slot = m_slots[m_frameIndex];

		if (slot.currSize != m_targetSize)
		{
			DestroySlot(m_frameIndex);
			CreateSlot(m_frameIndex);

			if (!slot.pTargetView || !slot.pDepthView)
				return;
		}

		AssetService* pAssetService = GetEngine()->RequestService<AssetService>();
		MeshAssetStreamer* pMeshStreamer = pAssetService->FindLoader<MeshAssetStreamer>(Reflect::TypeOf<MeshAsset>());

		RenderPassFrame currentFrame = {};
		currentFrame.cameraPosition = pCamera->m_worldPosition;
		currentFrame.viewMatrix = pCamera->m_view;
		currentFrame.projMatrix = pCamera->m_projection;
		currentFrame.viewProjMatrix = pCamera->m_viewProjection;
		currentFrame.targetSize = m_targetSize;
		currentFrame.frameIndex = m_frameIndex;
		currentFrame.pEngineFrame = &const_cast<EngineFrame&>(ctx);
		currentFrame.pColorTarget = slot.pTargetTexture.Get();
		currentFrame.pColorView = slot.pTargetView.Get();
		currentFrame.pCmdList = slot.pTargetCmd.Get();
		currentFrame.pDepthTarget = slot.pTargetDepth.Get();
		currentFrame.pDepthView = slot.pDepthView.Get();
		currentFrame.pIndexBuffer = pMeshStreamer->GetIndexBuffer();
		currentFrame.vertexBufferIndex = pMeshStreamer->GetVertexBufferView();

		currentScene.ForEach<TransformComponent, MeshComponent>([&](EntityHandle entity, TransformComponent& transComp, MeshComponent& meshComp)
			{
				if (meshComp.m_hideInRender)
					return;

				if (!meshComp.m_meshHandle.GetId().IsValid())
					return;

				MeshAsset* pMesh = meshComp.m_meshHandle.GetAsset();
				if (!pMesh)
				{
					auto* pAsset = pAssetService->FindAsset(meshComp.m_meshHandle.GetId());
					if (!pAsset)
					{
						Terminal::Error(StringHelper::GetTypeName(this), "{} is not a valid living mesh in the system", meshComp.m_meshHandle.GetId().ToString());
						return;
					}

					meshComp.m_meshHandle.SetAsset(pAsset);
				}

				pMesh = meshComp.m_meshHandle.GetAsset();
				if (pMesh->GetResidency() != AssetResidency::Resident)
				{
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
				}
			});

		m_fence->WaitCPU(slot.fenceValue);

		RenderFrameData frameData = {};
		frameData.view = pCamera->m_view;
		frameData.proj = pCamera->m_projection;
		frameData.viewProj = pCamera->m_viewProjection;
		frameData.cameraPosition = pCamera->m_worldPosition;
		frameData.time = f32(ctx.GetTotalSeconds());

		std::memcpy(m_frameDataBuffer->GetMappedData() + u64(m_frameIndex) * m_frameDataStride, &frameData, sizeof(RenderFrameData));
		currentFrame.frameDataIndex = m_frameDataViews[m_frameIndex];

		slot.pTargetPool->Reset();
		slot.pTargetCmd->Start();
		slot.pTargetCmd->BindDescriptorHeaps(m_resourceHeap, m_samplerHeap);
		{
			RHI::TextureBarrier beginBarrier = {};
			beginBarrier.pTexture = slot.pTargetTexture.Get();
			beginBarrier.before = slot.currState;
			beginBarrier.after = slot.currState = RHI::ResourceState::RenderTarget;
			beginBarrier.baseMip = 0;
			beginBarrier.baseLayer = 0;
			beginBarrier.mipCount = 1;
			beginBarrier.layerCount = 1;

			RHI::TextureBarrier depthBarrier = {};
			depthBarrier.pTexture = slot.pTargetDepth.Get();
			depthBarrier.before = RHI::ResourceState::Undefined;
			depthBarrier.after = RHI::ResourceState::DepthWrite;
			depthBarrier.baseMip = 0;
			depthBarrier.baseLayer = 0;
			depthBarrier.mipCount = 1;
			depthBarrier.layerCount = 1;

			slot.pTargetCmd->Barrier({ beginBarrier, depthBarrier });
		}

		for (auto* pPass : m_renderPasses)
			pPass->RenderFrame(currentFrame);

		{
			RHI::TextureBarrier endBarrier = {};
			endBarrier.pTexture = slot.pTargetTexture.Get();
			endBarrier.before = slot.currState;
			endBarrier.after = slot.currState = RHI::ResourceState::ShaderResource;
			endBarrier.baseMip = 0;
			endBarrier.baseLayer = 0;
			endBarrier.mipCount = 1;
			endBarrier.layerCount = 1;
			slot.pTargetCmd->Barrier({ endBarrier });
		}

		slot.pTargetCmd->End();

		m_device->Submit(RHI::QueueType::Graphics, { slot.pTargetCmd.Get() });
		slot.fenceValue = m_device->Signal(RHI::QueueType::Graphics, m_fence.Get());

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

			m_frameDataBuffer = nullptr;
		}

		for (auto* pPass : m_renderPasses)
			pPass->OnFinalize();

		for (auto* pPass : m_renderPasses)
			delete pPass;

		m_renderPasses.Clear();

		for (usize i = 0; i < m_slots.GetCount(); i++)
			DestroySlot(i);

		m_slots.Clear();

		m_fence = nullptr;

		m_resourceHeap = nullptr;
		m_samplerHeap = nullptr;
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

		RHI::CommandPoolDesc poolDesc = {};
		poolDesc.type = RHI::QueueType::Graphics;
		slot.pTargetPool = m_device->CreateRHICommandPool(poolDesc);

		if (slot.pTargetPool)
		{
			RHI::CommandListDesc listDesc = {};
			listDesc.pPool = slot.pTargetPool.Get();
			slot.pTargetCmd = m_device->CreateRHICommandList(listDesc);
		}

		RHI::TextureDesc colorDesc = {};
		colorDesc.width = m_targetSize.X();
		colorDesc.height = m_targetSize.Y();
		colorDesc.type = RHI::TextureType::Texture2D;
		colorDesc.format = RHI::TextureFormat::RGBA8_UNORM;
		colorDesc.usage = RHI::TextureUsage::RenderTarget | RHI::TextureUsage::Sampled;
		colorDesc.clearColor[3] = 1.0f;

		RHI::TextureDesc depthDesc = {};
		depthDesc.width = m_targetSize.X();
		depthDesc.height = m_targetSize.Y();
		depthDesc.type = RHI::TextureType::Texture2D;
		depthDesc.format = RHI::TextureFormat::D32_FLOAT;
		depthDesc.usage = RHI::TextureUsage::DepthStencil;
		depthDesc.clearDepth = 0.0f;

		slot.pTargetTexture = m_device->CreateRHITexture(colorDesc);
		slot.pTargetDepth = m_device->CreateRHITexture(depthDesc);

		if (!slot.pTargetCmd || !slot.pTargetTexture || !slot.pTargetDepth)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Slot {} could not be created with {}x{}", index, m_targetSize.X(), m_targetSize.Y());
			return;
		}

		slot.pTargetTexture->SetDebugName("Scene - ColorTarget");
		slot.pTargetDepth->SetDebugName("Scene - DepthTarget");

		RHI::TextureViewDesc colorViewDesc = {};
		colorViewDesc.pTexture = slot.pTargetTexture.Get();
		colorViewDesc.type = RHI::TextureViewType::Texture2D;
		slot.pTargetView = m_device->CreateRHITextureView(colorViewDesc);

		RHI::TextureViewDesc depthViewDesc = {};
		depthViewDesc.pTexture = slot.pTargetDepth.Get();
		depthViewDesc.type = RHI::TextureViewType::Texture2D;
		slot.pDepthView = m_device->CreateRHITextureView(depthViewDesc);

		if (!slot.pTargetView || !slot.pDepthView)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Slot {} views could not be created", index);
			return;
		}

		slot.shaderView = m_resourceHeap->Allocate();
		if (slot.shaderView != kInvalid32)
			m_resourceHeap->Write(slot.shaderView, RHI::DescriptorType::Texture, slot.pTargetView.Get());

		slot.currState = RHI::ResourceState::Undefined;
		slot.currSize = m_targetSize;
		slot.fenceValue = 0;
	}

	void RenderSystem::DestroySlot(usize index)
	{
		RenderSlot& slot = m_slots[index];

		m_device->WaitIdle();

		if (slot.shaderView != kInvalid32)
			m_resourceHeap->Free(slot.shaderView);

		slot = RenderSlot();
	}

	u64 RenderSystem::GetOutputView() const
	{
		if (!m_slots[m_frameIndex].pTargetTexture || m_slots[m_frameIndex].shaderView == kInvalid32)
			return kInvalid64;

		return m_resourceHeap->GetGpuHandle(m_slots[m_frameIndex].shaderView);
	}
}