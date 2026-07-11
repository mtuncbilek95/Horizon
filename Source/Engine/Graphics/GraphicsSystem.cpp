#include "GraphicsSystem.h"

#include <Engine/Core/Engine.h>
#include <Engine/Window/WindowSystem.h>

#include <Runtime/PAL/Window/Window.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Queue/GfxQueue.h>
#include <Runtime/RHI/Fence/GfxFence.h>
#include <Runtime/RHI/Command/GfxCommandList.h>
#include <Runtime/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/Definitions/Allocator.h>

namespace Horizon
{
	SystemReport GraphicsSystem::OnAttach(Engine* pEngine)
	{
		System::OnAttach(pEngine);

		auto* pWindowSub = m_engine->TryGetSystem<WindowSystem>();
		if(!pWindowSub)
			return SystemReport("Failed to get WindowSystem. Nothing will work...");

		m_device = CreateGfxDevice();
		if (!m_device)
			return SystemReport("Failed to create GfxDevice");

		m_graphicsQueue = m_device->CreateQueue(GfxQueueType::Graphics);
		if (!m_graphicsQueue)
			return SystemReport("Failed to create GfxQueue(Graphics)");

		m_computeQueue = m_device->CreateQueue(GfxQueueType::Compute);
		if (!m_computeQueue)
			return SystemReport("Failed to create GfxQueue(Compute)");

		m_transferQueue = m_device->CreateQueue(GfxQueueType::Transfer);
		if (!m_transferQueue)
			return SystemReport("Failed to create GfxQueue(Transfer)");

		Terminal::Debug("GraphicsSystem", "Device, Graphics Queue, Compute Queue and Transfer Queue has been initialized!");
		return SystemReport();
	}

	void GraphicsSystem::OnDetach()
	{
		Allocator::Delete(m_graphicsQueue);
		Allocator::Delete(m_computeQueue);
		Allocator::Delete(m_transferQueue);

		Allocator::Delete(m_device);
	}

	void GraphicsSystem::GetInitializeOrder(OrderRules& rules) const
	{
		Requires<WindowSystem>(rules.after);
	}

	void GraphicsSystem::GetExecutionOrder(OrderRules& rules) const
	{
		Requires<WindowSystem>(rules.after);
	}

}
