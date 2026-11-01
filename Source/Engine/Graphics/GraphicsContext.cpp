#include "GraphicsContext.h"

#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleGraph.h>
#include <Engine/Window/WindowService.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/PAL/Window/Window.h>

namespace Horizon::Engine
{
	ModuleReport GraphicsContext::OnInitialize()
	{
		// TODO: After sometime, capacity that comes from 
		// project settings would be awesome. Even deviceDesc
		// can be imported from some shit.

		// Get window service to use on swapchain
		auto* pWindowSub = GetEngine()->RequestService<WindowService>();
		if (!pWindowSub)
			return ModuleReport("Failed to get WindowService. Nothing will work...");

		RHI::DeviceDesc deviceDesc = {};
#if defined(HORIZON_DEBUG)
		deviceDesc.enableDebug = true;
		deviceDesc.enableGpuValidation = true;
#endif

		// Create device
		m_device = RHI::CreateRHIDevice(deviceDesc);
		if (!m_device)
			return ModuleReport("Failed to create Device");

		RHI::DescriptorHeapDesc heapDesc = {};

		heapDesc.type = RHI::DescriptorHeapType::Resource;
		heapDesc.capacity = 1 << 16;
		m_resourceHeap = m_device->CreateRHIDescriptorHeap(heapDesc);
		if (!m_resourceHeap)
			return ModuleReport("Failed to create DescriptorHeap(Resource)");

		heapDesc.type = RHI::DescriptorHeapType::Sampler;
		heapDesc.capacity = 1 << 11;
		m_samplerHeap = m_device->CreateRHIDescriptorHeap(heapDesc);
		if (!m_samplerHeap)
			return ModuleReport("Failed to create DescriptorHeap(Sampler)");

		m_globalLayout = m_device->CreateRHIGlobalPipelineLayout();
		if (!m_globalLayout)
			return ModuleReport("Failed to create global PipelineLayout");

		PAL::WindowRect windowRect = pWindowSub->GetWindow()->GetRect();

		// Create swapchain
		RHI::SwapchainDesc swapDesc = {};
		swapDesc.windowHandle = pWindowSub->GetWindow()->GetOSHandle();
		swapDesc.imageCount = 3;
		swapDesc.width = windowRect.width;
		swapDesc.height = windowRect.height;
		swapDesc.presentMode = RHI::PresentMode::Vsync;
		swapDesc.format = RHI::TextureFormat::RGBA8_UNORM;
		m_swapchain = m_device->CreateRHISwapchain(swapDesc);
		if (!m_swapchain)
			return ModuleReport("Failed to create Swapchain");

		Terminal::Debug(StringHelper::GetTypeName(this), "Device, descriptor heaps, global layout and swapchain has been initialized!");

		return ModuleReport();
	}

	void GraphicsContext::OnFinalize()
	{
		if (m_device)
			m_device->WaitIdle();

		m_swapchain = nullptr;
		m_globalLayout = nullptr;
		m_samplerHeap = nullptr;
		m_resourceHeap = nullptr;
		m_device = nullptr;
	}

	void GraphicsContext::DeclareDependencies(ModuleGraph& graph)
	{
		graph.Requires<WindowService>();
	}

	void GraphicsContext::OnLibraryRegistered(const ReflectionLibrary& library)
	{
	}

	void GraphicsContext::OnLibraryUnregistered(const ReflectionLibrary& library)
	{
	}
}