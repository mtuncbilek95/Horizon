#pragma once

#include <Runtime/Graphics/RHI/Buffer/GfxBuffer.h>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Horizon
{
	class GfxVkBuffer final : public GfxBuffer
	{
	public:
		GfxVkBuffer(const GfxBufferDesc& desc, GfxDevice* pDevice);
		~GfxVkBuffer() final;

		usize DeviceAddress() const final;
		void* Buffer() const final;
		void* MappedData() const final;
		usize MappedSize() const final;

		void Update(const ReadArray<u8>& data) final;
		void Update(void* buffer, usize size) final;
		void Map(const ReadArray<u8>& data) final;
		void Map(void* buffer, usize size) final;
		void Unmap() override final;

	private:
		VkBuffer m_buffer;
		VmaAllocation m_allocation;
		VmaAllocationInfo m_allocationInfo;
		VkDeviceAddress m_deviceAddress;

		VkPhysicalDeviceMemoryProperties m_memProps;
	};
}
