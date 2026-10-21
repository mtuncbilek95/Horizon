#include "RenderSystem.h"

#include <Engine/World/Components/Physics/TransformComponent.h>
#include <Engine/World/Components/Rendering/CameraComponent.h>
#include <Engine/World/Components/Rendering/MeshComponent.h>
#include <Engine/World/Components/PostProcess/FogComponent.h>
#include <Engine/World/Systems/Rendering/CameraSystem.h>
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

		if (m_resourceHeap == nullptr || m_colorHeap == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "Descriptor heaps are unavailable, render system stays down");
			return false;
		}

		ResizeImage({ 1280, 720 });
		m_slots.Resize(GraphicsContext::MaxFramesInFlight);
		for (u32 i = 0; i < GraphicsContext::MaxFramesInFlight; i++)
		{
			if (!RecreateSlot(i))
				return false;
		}

		m_fence = m_device->CreateFence();

		return true;
	}

	void RenderSystem::OnExecute(const EngineFrame& ctx, Scene& currentScene)
	{
		Math::Mat4f viewProj = Math::Mat4f::Identity();
		Math::Vec3f camPos = Math::Vec3f::Zero();

		CameraSystem* pCameraSystem = RequestSystem<CameraSystem>();
		CameraComponent* pCamera = nullptr;

		// Find the correct camera
		if (pCameraSystem != nullptr && pCameraSystem->GetActiveCamera().IsValid())
			pCamera = currentScene.FindComponent<CameraComponent>(pCameraSystem->GetActiveCamera());

		if (pCamera != nullptr && pCamera->m_targetScreen.X() >= 1.0f && pCamera->m_targetScreen.Y() >= 1.0f)
		{
			viewProj = pCamera->m_viewProjection;
			camPos = pCamera->m_worldPosition;
			ResizeImage({ u32(pCamera->m_targetScreen.X()), u32(pCamera->m_targetScreen.Y()) });
		}

		// Resize if needed
		RenderSlot& slot = m_slots[m_frameIndex];
		if (slot.currSize != m_targetSize)
		{
			if (!RecreateSlot(m_frameIndex))
			{
				Terminal::Error(StringOps::GetName(this), "Failed to resize color target slot {}", m_frameIndex);
				return;
			}
		}

		m_fence->WaitCPU(slot.fenceValue);

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
		RHI::GfxRenderBeginDesc renderDesc = RHI::GfxRenderBeginDesc()
			.AddColorTarget(slot.pTargetTexture, RHI::GfxLoadOp::Clear, { 0.0f, 0.0f, 0.0f, 1.f })
			/*.SetDepth(sFunDepthTextures[m_frameIndex], RHI::GfxLoadOp::Clear, 0.0f)*/
			.SetSize(slot.pTargetTexture->GetDesc().width, slot.pTargetTexture->GetDesc().height);

		slot.pTargetCmd->BeginRendering(renderDesc);
		slot.pTargetCmd->SetScissor({ 0, 0, (i32)slot.pTargetTexture->GetDesc().width, (i32)slot.pTargetTexture->GetDesc().height });
		slot.pTargetCmd->SetViewport({ 0, 0, (f32)slot.pTargetTexture->GetDesc().width, (f32)slot.pTargetTexture->GetDesc().height, 0.f, 1.f });

		slot.pTargetCmd->EndRendering();
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
		m_device->WaitIdle();

		Memory::Allocator::Delete(m_fence);

		for (usize i = 0; i < m_slots.GetCount(); i++)
		{
			ClearSlot(i);
			Memory::Allocator::Delete(m_slots[i].pTargetCmd);
		}

		if (m_colorHeap)
		{
			Memory::Allocator::Delete(m_colorHeap);
			m_colorHeap = nullptr;
		}

		if (m_resourceHeap)
		{
			Memory::Allocator::Delete(m_resourceHeap);
			m_resourceHeap = nullptr;
		}

	}

	void RenderSystem::OnGroupsChanged(SystemGroup previous, SystemGroup current)
	{
	}

	u64 RenderSystem::GetSceneView() const
	{
		if (!m_slots[m_frameIndex].pTargetTexture)
			return kInvalid64;

		return m_resourceHeap->GetGpuHandle(m_slots[m_frameIndex].shaderView);
	}

	void RenderSystem::ResizeImage(const Math::Vec2u& imgSize)
	{
		u32 width = Math::Max(1u, imgSize.X());
		u32 height = Math::Max(1u, imgSize.Y());

		if (width == m_targetSize.X() || height == m_targetSize.Y())
			return;

		m_targetSize = { width, height };
	}

	b8 RenderSystem::RecreateSlot(u32 imageIndex)
	{
		RenderSlot& slot = m_slots[imageIndex];

		if (slot.currSize == m_targetSize)
		{
			Terminal::Error(StringOps::GetName(this), "It should not be able to come here");
			return false;
		}

		m_device->WaitIdle();

		if (!ClearSlot(imageIndex))
			return false;

		if (slot.pTargetCmd == nullptr)
			slot.pTargetCmd = m_device->CreateCommandList(RHI::GfxQueueType::Graphics);

		RHI::GfxTextureDesc texDesc = {};
		texDesc.width = m_targetSize.X();
		texDesc.height = m_targetSize.Y();
		texDesc.type = RHI::GfxTextureType::Tex2D;
		texDesc.format = RHI::GfxTextureFormat::RGBA8_UNORM;
		texDesc.usage = RHI::GfxTextureUsage::RenderTarget | RHI::GfxTextureUsage::Sampled;
		texDesc.clearColor = { 0.0f, 0.0f, 0.0f, 1.f };
		texDesc.format = RHI::GfxTextureFormat::RGBA8_UNORM;

		slot.pTargetTexture = m_device->CreateTexture(texDesc);
		if (slot.pTargetTexture == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "Scene color target {}x{} could not be created",
				m_targetSize.X(), m_targetSize.Y());
			return false;
		}

		slot.pTargetTexture->SetDebugName("Scene - RenderTarget");

		slot.renderTargetView = m_colorHeap->CreateRenderTargetView(slot.pTargetTexture);
		slot.shaderView = m_resourceHeap->CreateShaderView(slot.pTargetTexture);

		slot.currSize = m_targetSize;
		slot.currState = RHI::GfxResourceState::Common;
		slot.fenceValue = 0;

		return true;
	}

	b8 RenderSystem::ClearSlot(u32 imageIndex)
	{
		RenderSlot& slot = m_slots[imageIndex];

		if (!slot.pTargetTexture)
			return true;

		m_colorHeap->Free(slot.renderTargetView);
		m_resourceHeap->Free(slot.shaderView);
		slot.renderTargetView = kInvalid32;
		slot.shaderView = kInvalid32;

		Memory::Allocator::Delete(slot.pTargetTexture);
		slot.pTargetTexture = nullptr;

		slot.fenceValue = 0;

		return true;
	}
}