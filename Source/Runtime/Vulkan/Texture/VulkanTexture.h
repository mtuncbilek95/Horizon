#pragma once

#include <Runtime/RHI/Texture/Texture.h>

#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>

namespace Horizon::RHI
{
	class VulkanTexture final : public Texture
	{
		friend class VulkanDevice;
		friend class VulkanSwapchain;
	public:
		~VulkanTexture() final;

		void SetDebugName(const c8* pName) final;

		VkImage Handle() const { return m_image; }
		VmaAllocation Allocation() const { return m_allocation; }

		VulkanSwapchain* GetOwnerSwapchain() const { return m_ownerSwapchain; }
		VkSemaphore GetCmdBuffReleaseSemaphore() const { return m_cmdBuffReleaseSemaphore; }
	private:
		VkImage m_image = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;

		VulkanSwapchain* m_ownerSwapchain = nullptr;
		// for swapchain synchronization, only valid for swapchain images
		VkSemaphore m_cmdBuffReleaseSemaphore = VK_NULL_HANDLE;
	};
}