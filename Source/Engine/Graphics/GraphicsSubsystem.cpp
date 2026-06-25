#include "GraphicsSubsystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowSubsystem.h>

#include <Runtime/PAL/Window/Window.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Queue/GfxQueue.h>
#include <Runtime/RHI/Fence/GfxFence.h>
#include <Runtime/RHI/Command/GfxCommandList.h>
#include <Runtime/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/Definitions/Allocator.h>

namespace Horizon
{
	EngineReport GraphicsSubsystem::OnAttach(Engine* pEngine)
	{
		Subsystem::OnAttach(pEngine);

		auto* pWindowSub = m_engine->TryGetSubsystem<WindowSubsystem>();
		if(!pWindowSub)
			return EngineReport("Failed to get WindowSubsystem. Nothing will work...");

		m_device = CreateGfxDevice();
		if (!m_device)
			return EngineReport("Failed to create GfxDevice");

		m_graphicsQueue = m_device->CreateQueue(GfxQueueType::Graphics);
		if (!m_graphicsQueue)
			return EngineReport("Failed to create GfxQueue(Graphics)");

		m_computeQueue = m_device->CreateQueue(GfxQueueType::Compute);
		if (!m_computeQueue)
			return EngineReport("Failed to create GfxQueue(Compute)");

		m_transferQueue = m_device->CreateQueue(GfxQueueType::Transfer);
		if (!m_transferQueue)
			return EngineReport("Failed to create GfxQueue(Transfer)");

		WindowRect windowRect = pWindowSub->GetWindow()->GetRect();

		GfxSwapchainDesc swapDesc = {};
		swapDesc.pWindowHandle = (void*)pWindowSub->GetWindow()->GetOSHandle();
		swapDesc.imageCount = 3;
		swapDesc.width = windowRect.width;
		swapDesc.height = windowRect.height;
		swapDesc.vSync = true;
		swapDesc.bAllowTearing = false;
		m_swapchain = m_device->CreateSwapchain(swapDesc, m_graphicsQueue);
		if (!m_swapchain)
			return EngineReport("Failed to create GfxSwapchain");

		return EngineReport();
	}

	void GraphicsSubsystem::OnDetach()
	{
		Allocator::Delete(m_swapchain);

		Allocator::Delete(m_graphicsQueue);
		Allocator::Delete(m_computeQueue);
		Allocator::Delete(m_transferQueue);

		Allocator::Delete(m_device);
	}

	void GraphicsSubsystem::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<WindowSubsystem>(rules.after);
	}

	void GraphicsSubsystem::GetExecutionOrder(OrderRules& rules) const
	{
		Requires<WindowSubsystem>(rules.after);
	}

}
