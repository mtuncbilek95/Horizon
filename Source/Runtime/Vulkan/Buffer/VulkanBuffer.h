#pragma once

#include <Runtime/RHI/Buffer/Buffer.h>

#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>

namespace Horizon::RHI
{
	class VulkanBuffer final : public Buffer
	{
		friend class VulkanDevice;
	public:
		~VulkanBuffer() final;

		void SetDebugName(const c8* pName) final;

		VkBuffer Handle() const { return m_buffer; }
		VmaAllocation Allocation() const { return m_allocation; }

	private:
		VkBuffer m_buffer = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
	};
}