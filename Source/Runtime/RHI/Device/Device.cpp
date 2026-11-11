#include "Device.h"



#include <Runtime/RHI/Pipeline/GlobalPipelineLayout.h>
#include <Runtime/RHI/Pipeline/PipelineLayout.h>

#include <Runtime/D3D12/Device/D3D12Device.h>
#include <Runtime/Vulkan/Device/VulkanDevice.h>

namespace Horizon::RHI
{
	UniquePtr<Device> CreateRHIDevice(DeviceDesc& desc)
	{
		switch (desc.api)
		{
		case Backend::DX12:
		{
			UniquePtr<D3D12Device> pDevice = MakeUnique<D3D12Device>();
			if (pDevice->Initialize(desc))
				return pDevice;

			return nullptr;
		}
		case Backend::Vulkan:
		{
			UniquePtr<VulkanDevice> pDevice = MakeUnique<VulkanDevice>();
			if (pDevice->Initialize(desc))
				return pDevice;

			return nullptr;
		}
		default:
			return nullptr;
		}

		return nullptr;
	}

	UniquePtr<PipelineLayout> Device::CreateRHIGlobalPipelineLayout()
	{
		PipelineLayoutDesc desc = {};
		desc.pushConstantSize = kGlobalPushConstantSize;
		desc.bindless = true;

		return CreateRHIPipelineLayout(desc);
	}
}