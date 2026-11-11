#pragma once

#include <Runtime/RHI/Swapchain/Swapchain.h>

#include <volk/volk.h>

namespace Horizon::RHI
{
	class VulkanSwapchain final : public Swapchain
	{
		friend class VulkanDevice;
	public:
		~VulkanSwapchain() final;

		b8 AcquireNextImage() final;
		void Present() final;
		void Resize(u32 width, u32 height) final;

		void SetDebugName(const c8* pName) final;

		VkSwapchainKHR Handle() const { return m_swapchain; }
	private:
		b8 CreateImages();
		void ReleaseImages();
		void ReCreateSwapchain();

	private:
		VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VkPresentModeKHR m_vkPresentMode = VK_PRESENT_MODE_FIFO_KHR;

        List<VkSemaphore> m_presentSemaphores{};
        VkFence m_acquireFence = VK_NULL_HANDLE;

		u32 m_frameIndex = 0;
		b8 m_isAcquired;
	};
}
