#include "GraphicsContext.h"

#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Window/WindowService.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/PAL/Window/Window.h>
#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/RHI/Queue/GfxQueue.h>
#include <Runtime/RHI/Fence/GfxFence.h>
#include <Runtime/RHI/Command/GfxCommandList.h>
#include <Runtime/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/Definitions/Allocator.h>

namespace Horizon::Engine
{
	ModuleReport GraphicsContext::OnInitialize()
	{
		auto* pWindowSub = GetEngine()->RequestService<WindowService>();
		if (!pWindowSub)
			return ModuleReport("Failed to get WindowService. Nothing will work...");

		m_device = CreateGfxDevice(GfxDeviceDesc());
		if (!m_device)
			return ModuleReport("Failed to create GfxDevice");

		m_graphicsQueue = m_device->CreateQueue(GfxQueueType::Graphics);
		if (!m_graphicsQueue)
			return ModuleReport("Failed to create GfxQueue(Graphics)");

		m_computeQueue = m_device->CreateQueue(GfxQueueType::Compute);
		if (!m_computeQueue)
			return ModuleReport("Failed to create GfxQueue(Compute)");

		m_transferQueue = m_device->CreateQueue(GfxQueueType::Transfer);
		if (!m_transferQueue)
			return ModuleReport("Failed to create GfxQueue(Transfer)");

		PAL::WindowRect windowRect = pWindowSub->GetWindow()->GetRect();

		GfxSwapchainDesc swapDesc = {};
		swapDesc.pWindowHandle = (void*)pWindowSub->GetWindow()->GetOSHandle();
		swapDesc.imageCount = 3;
		swapDesc.width = windowRect.width;
		swapDesc.height = windowRect.height;
		swapDesc.vSync = true;
		swapDesc.bAllowTearing = false;
		m_swapchain = m_device->CreateSwapchain(swapDesc, m_graphicsQueue);
		if (!m_swapchain)
			return ModuleReport("Failed to create GfxSwapchain");

		Terminal::Debug(StringOps::GetName(this), "Device, Graphics Queue, Compute Queue and Transfer Queue has been initialized!");

		return ModuleReport();
	}

	void GraphicsContext::OnFinalize()
	{
		Memory::Allocator::Delete(m_graphicsQueue);
		Memory::Allocator::Delete(m_computeQueue);
		Memory::Allocator::Delete(m_transferQueue);

		Memory::Allocator::Delete(m_device);
	}

	void GraphicsContext::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<WindowService>();
	}
}