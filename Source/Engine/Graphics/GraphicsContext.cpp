#include "GraphicsContext.h"

#include <Engine/Core/Application.h>
#include <Engine/Window/WindowSystem.h>

#include <Runtime/PAL/Window/Window.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Queue/GfxQueue.h>
#include <Runtime/RHI/Fence/GfxFence.h>
#include <Runtime/RHI/Command/GfxCommandList.h>
#include <Runtime/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/Definitions/Allocator.h>

namespace Horizon::Engine
{
	AppReport GraphicsContext::OnAttach(Application* pEngine)
	{
		Context::OnAttach(pEngine);

		auto* pWindowSub = m_engine->TryGetSystem<WindowSystem>();
		if(!pWindowSub)
			return AppReport("Failed to get WindowSystem. Nothing will work...");

		m_device = CreateGfxDevice(GfxDeviceDesc());
		if (!m_device)
			return AppReport("Failed to create GfxDevice");

		m_graphicsQueue = m_device->CreateQueue(GfxQueueType::Graphics);
		if (!m_graphicsQueue)
			return AppReport("Failed to create GfxQueue(Graphics)");

		m_computeQueue = m_device->CreateQueue(GfxQueueType::Compute);
		if (!m_computeQueue)
			return AppReport("Failed to create GfxQueue(Compute)");

		m_transferQueue = m_device->CreateQueue(GfxQueueType::Transfer);
		if (!m_transferQueue)
			return AppReport("Failed to create GfxQueue(Transfer)");

		Terminal::Debug("GraphicsContext", "Device, Graphics Queue, Compute Queue and Transfer Queue has been initialized!");
		return AppReport();
	}

	void GraphicsContext::OnDetach()
	{
		Memory::Allocator::Delete(m_graphicsQueue);
		Memory::Allocator::Delete(m_computeQueue);
		Memory::Allocator::Delete(m_transferQueue);

		Memory::Allocator::Delete(m_device);
	}

	void GraphicsContext::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<WindowSystem>(rules.after);
	}
}
