#include "GraphicsContext.h"

#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleDependency.h>

#include <Runtime/Containers/StringHelper.h>

namespace Horizon::Engine
{
	ModuleReport GraphicsContext::OnInitialize()
	{
		// TODO: After sometime, capacity that comes from 
		// project settings would be awesome. Even deviceDesc
		// can be imported from some shit.

		RHI::DeviceDesc deviceDesc = {};
#if defined(HORIZON_DEBUG)
		deviceDesc.enableDebug = false;
		deviceDesc.enableGpuValidation = false;
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

		Terminal::Debug(StringHelper::GetTypeName(this), "Device, descriptor heaps and global layout has been initialized!");

		return ModuleReport();
	}

	void GraphicsContext::OnFinalize()
	{
		if (m_device)
			m_device->WaitIdle();

		m_globalLayout = nullptr;
		m_samplerHeap = nullptr;
		m_resourceHeap = nullptr;
		m_device = nullptr;
	}

	void GraphicsContext::DeclareDependencies(ModuleDependency& graph)
	{
	}
}