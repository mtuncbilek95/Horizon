#include "GraphicsContext.h"

#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Window/WindowService.h>

#include <Runtime/Containers/StringOps.h>
#include <Runtime/PAL/Window/Window.h>
#include <Runtime/Definitions/Allocator.h>

namespace Horizon::Engine
{
	ModuleReport GraphicsContext::OnInitialize()
	{
		// TODO: After sometime, capacity that comes from 
		// project settings would be awesome.

		// Get window service to use on swapchain
		auto* pWindowSub = GetEngine()->RequestService<WindowService>();
		if (!pWindowSub)
			return ModuleReport("Failed to get WindowService. Nothing will work...");

		RHI::GfxDeviceDesc deviceDesc = {};
#if defined(HORIZON_DEBUG)
		deviceDesc.enableDebugLayer = true;
#endif

		// Create device
		m_device = RHI::CreateDevice(deviceDesc);
		if (!m_device)
			return ModuleReport("Failed to create GfxDevice");

		// Create resource heap
		RHI::GfxDescriptorHeapDesc resourceHeapDesc = {};
		resourceHeapDesc.capacity = 1 << 16;
		resourceHeapDesc.shaderVisible = true;
		resourceHeapDesc.type = RHI::GfxDescriptorHeapType::Resource;
		m_resourceHeap = m_device->CreateDescriptorHeap(resourceHeapDesc);
		if (!m_resourceHeap)
			return ModuleReport("Failed to create GfxDescriptorHeap(Resource)");

		// Create rtv heap
		RHI::GfxDescriptorHeapDesc colorHeapDesc = {};
		colorHeapDesc.capacity = 1 << 10;
		colorHeapDesc.shaderVisible = false;
		colorHeapDesc.type = RHI::GfxDescriptorHeapType::Color;
		m_colorHeap = m_device->CreateDescriptorHeap(colorHeapDesc);
		if (!m_colorHeap)
			return ModuleReport("Failed to create GfxDescriptorHeap(Color)");

		// Create queues
		m_graphicsQueue = m_device->CreateQueue(RHI::GfxQueueType::Graphics);
		if (!m_graphicsQueue)
			return ModuleReport("Failed to create GfxQueue(Graphics)");

		m_computeQueue = m_device->CreateQueue(RHI::GfxQueueType::Compute);
		if (!m_computeQueue)
			return ModuleReport("Failed to create GfxQueue(Compute)");

		m_transferQueue = m_device->CreateQueue(RHI::GfxQueueType::Transfer);
		if (!m_transferQueue)
			return ModuleReport("Failed to create GfxQueue(Transfer)");

		PAL::WindowRect windowRect = pWindowSub->GetWindow()->GetRect();

		// Create swapchain
		RHI::GfxSwapchainDesc swapDesc = {};
		swapDesc.pWindowHandle = (void*)pWindowSub->GetWindow()->GetOSHandle();
		swapDesc.imageCount = 3;
		swapDesc.width = windowRect.width;
		swapDesc.height = windowRect.height;
		swapDesc.presentMode = RHI::GfxPresentMode::Vsync;
		swapDesc.pColorHeap = m_colorHeap;
		m_swapchain = m_device->CreateSwapchain(swapDesc, m_graphicsQueue);
		if (!m_swapchain)
			return ModuleReport("Failed to create GfxSwapchain");

		Terminal::Debug(StringOps::GetName(this), "Device, Graphics Queue, Compute Queue and Transfer Queue has been initialized!");

		return ModuleReport();
	}

	void GraphicsContext::OnFinalize()
	{
		m_device->WaitIdle();

		Memory::Allocator::Delete(m_swapchain);

		Memory::Allocator::Delete(m_graphicsQueue);
		Memory::Allocator::Delete(m_computeQueue);
		Memory::Allocator::Delete(m_transferQueue);

		Memory::Allocator::Delete(m_colorHeap);
		Memory::Allocator::Delete(m_resourceHeap);

		Memory::Allocator::Delete(m_device);
	}

	void GraphicsContext::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<WindowService>();
	}
}