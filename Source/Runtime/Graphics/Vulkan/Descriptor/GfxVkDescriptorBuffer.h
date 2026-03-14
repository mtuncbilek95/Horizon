#pragma once

#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorBuffer.h>

#include <vma/vk_mem_alloc.h>

namespace Horizon
{
	class GfxVkDescriptorBuffer final : public GfxDescriptorBuffer
	{
	public:
		GfxVkDescriptorBuffer(const GfxDescriptorBufferDesc& desc, GfxDevice* pDevice);
		~GfxVkDescriptorBuffer() override final;

		void RequestPayload(GfxObject* pObject, usize offsetInBytes) final;
		void WriteDescriptor(usize offsetInBytes, const void* pDescriptorData, usize descriptorSize) final;
		u64 DeviceAddress() const final;
		void* MappedData() const final;

	private:
		VkBuffer m_buffer;
		VmaAllocation m_allocation;
		VmaAllocationInfo m_allocationInfo;
		u64 m_deviceAddress;
	};
}