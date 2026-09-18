#include "RenderSystem.h"

#include <Engine/World/Components/TransformComponent.h>
#include <Engine/World/Components/CameraComponent.h>
#include <Engine/World/Components/MeshComponent.h>

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
		m_resourceHeap = m_context->GetResourceHeap();
		m_colorHeap = m_context->GetColorHeap();
		m_queue = m_context->GetGraphicsQueue();

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
		RenderSlot& slot = m_slots[m_frameIndex];

		if (slot.currSize != m_targetSize)
		{
			if (!RecreateSlot(m_frameIndex))
			{
				Terminal::Error(StringOps::GetName(this), "Failed to resize color target slot {}", m_frameIndex);
				return;
			}
		}

		Math::Mat4f viewProj = Math::Mat4f::Identity();
		currentScene.ForEach<CameraComponent>([&](EntityHandle handl, CameraComponent& camMatrix)
			{
				viewProj = camMatrix.m_viewProjection;
			});

		m_colorHeap->Recycle();
		m_resourceHeap->Recycle();

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
			.AddColorTarget(slot.pTargetTexture, RHI::GfxLoadOp::Clear, { 0.39f, 0.58f, 0.92f, 1.f })
			.SetSize(slot.pTargetTexture->GetDesc().width, slot.pTargetTexture->GetDesc().height);

		slot.pTargetCmd->BeginRendering(renderDesc);
		slot.pTargetCmd->SetScissor({ 0, 0, (i32)slot.pTargetTexture->GetDesc().width, (i32)slot.pTargetTexture->GetDesc().height });
		slot.pTargetCmd->SetViewport({ 0, 0, (f32)slot.pTargetTexture->GetDesc().width, (f32)slot.pTargetTexture->GetDesc().height, 0.f, 1.f });

		currentScene.ForEach<MeshComponent, TransformComponent>([&](EntityHandle handl, MeshComponent& mesh, TransformComponent& worldMat)
			{
				MeshAsset* pAsset = mesh.m_meshId.GetAsset();

				// No asset, no call
				if (!pAsset)
					return;

				pAsset->BeginUse();
			});

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

		m_colorHeap->Recycle();
		m_resourceHeap->Recycle();

		Memory::Allocator::Delete(m_fence);

		for (usize i = 0; i < m_slots.GetCount(); i++)
		{
			ClearSlot(i);
			Memory::Allocator::Delete(m_slots[i].pTargetCmd);
		}
	}

	u64 RenderSystem::GetSceneView() const
	{
		if (!m_slots[m_frameIndex].pTargetTexture)
			return kInvalid64;

		return m_resourceHeap->GetGpuHandle(m_slots[m_frameIndex].pTargetTexture->GetShaderView());
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
		texDesc.clearColor = { 0.39f, 0.58f, 0.92f, 1.f };
		texDesc.format = RHI::GfxTextureFormat::RGBA8_UNORM;

		slot.pTargetTexture = m_device->CreateTexture(texDesc);

		if (slot.pTargetTexture == nullptr)
		{
			Terminal::Error(StringOps::GetName(this), "Scene color target {}x{} could not be created",
				m_targetSize.X(), m_targetSize.Y());
			return false;
		}

		slot.pTargetTexture->SetDebugName("Scene - RenderTarget");

		m_colorHeap->CreateRenderTargetView(slot.pTargetTexture);
		m_resourceHeap->CreateShaderView(slot.pTargetTexture);

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

		Memory::Allocator::Delete(slot.pTargetTexture);
		slot.pTargetTexture = nullptr;

		slot.fenceValue = 0;

		return true;
	}
}