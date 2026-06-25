#include "GraphicsModule.h"

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowModule.h>

#include <Runtime/PAL/Window/Window.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Queue/GfxQueue.h>
#include <Runtime/RHI/Fence/GfxFence.h>
#include <Runtime/RHI/Command/GfxCommandList.h>
#include <Runtime/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/Definitions/Allocator.h>

namespace Horizon
{
	void GraphicsModule::OnAttach(Engine* pEngine)
	{
		Subsystem::OnAttach(pEngine);

		auto* windowModule = m_engine->TryGetModule<WindowModule>();

		m_device = CreateGfxDevice();

		m_graphicsQueue = m_device->CreateQueue(GfxQueueType::Graphics);
		m_computeQueue = m_device->CreateQueue(GfxQueueType::Compute);
		m_transferQueue = m_device->CreateQueue(GfxQueueType::Transfer);

		WindowRect windowRect = windowModule->GetWindow()->GetRect();
		m_width = windowRect.width;
		m_height = windowRect.height;

		GfxSwapchainDesc swapDesc = {};
		swapDesc.pWindowHandle = (void*)windowModule->GetWindow()->GetOSHandle();
		swapDesc.imageCount = 3;
		swapDesc.width = m_width;
		swapDesc.height = m_height;
		swapDesc.vSync = true;
		swapDesc.bAllowTearing = false;
		m_swapchain = m_device->CreateSwapchain(swapDesc, m_graphicsQueue);

		m_frameFence = m_device->CreateFence();
		for (u32 i = 0; i < MaxFramesInFlight; i++)
			m_frameCmds[i] = m_device->CreateCommandList(GfxQueueType::Graphics);
	}

	void GraphicsModule::OnSync()
	{
		const u32 slot = u32(m_frameIndex % MaxFramesInFlight);

		// Don't reuse this slot's command list until the GPU finished its last use.
		m_frameFence->WaitCPU(m_frameFenceValues[slot]);

		GfxCommandList* cmd = m_frameCmds[slot];
		GfxTexture* backbuffer = m_swapchain->GetBackbuffer(m_swapchain->GetCurrentIndex());

		cmd->Begin();

		GfxTextureBarrier toTarget = { backbuffer, GfxResourceState::Present, GfxResourceState::RenderTarget };
		cmd->Barrier(&toTarget, 1);

		GfxRenderBeginDesc beginDesc = {};
		beginDesc.addColorTarget(backbuffer, GfxLoadOp::Clear, { 0.10f, 0.10f, 0.12f, 1.0f }).setSize(m_width, m_height);
		cmd->BeginRendering(beginDesc);

		// TODO: composite scene texture + editor UI here.

		GfxTextureBarrier toPresent = { backbuffer, GfxResourceState::RenderTarget, GfxResourceState::Present };
		cmd->Barrier(&toPresent, 1);

		cmd->End();

		m_graphicsQueue->Submit(&cmd, 1);
		m_frameFenceValues[slot] = m_graphicsQueue->Signal(m_frameFence);

		m_swapchain->Present();

		m_frameIndex++;
		m_device->SetFrameIndex(m_frameIndex);
		m_device->FlushPendingDeletes(m_frameIndex);
	}

	void GraphicsModule::OnDetach()
	{
		// Drain the GPU before tearing anything down.
		const u64 last = m_graphicsQueue->Signal(m_frameFence);
		m_frameFence->WaitCPU(last);

		for (u32 i = 0; i < MaxFramesInFlight; i++)
			Allocator::Delete(m_frameCmds[i]);

		Allocator::Delete(m_frameFence);

		// Swapchain before device: its backbuffers free descriptors on the device.
		Allocator::Delete(m_swapchain);

		Allocator::Delete(m_graphicsQueue);
		Allocator::Delete(m_computeQueue);
		Allocator::Delete(m_transferQueue);

		Allocator::Delete(m_device);
	}
}
