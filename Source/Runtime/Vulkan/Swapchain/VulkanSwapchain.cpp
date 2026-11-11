#include "VulkanSwapchain.h"

#include <Runtime/Vulkan/Helpers/VulkanHelpers.h>
#include <Runtime/Vulkan/Texture/VulkanTexture.h>
#include <Runtime/Vulkan/Device/VulkanDevice.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

#include <algorithm>
#include <utility>

namespace Horizon::RHI
{
	VulkanSwapchain::~VulkanSwapchain()
	{
		ReleaseImages();
		auto* pVkDevice = (VulkanDevice*)m_ownerDevice;

		if (m_swapchain)
			vkDestroySwapchainKHR(pVkDevice->GetDevice(), m_swapchain, nullptr);

		if (m_surface)
			vkDestroySurfaceKHR(pVkDevice->GetInstance(), m_surface, nullptr);

		if (m_acquireFence)
			vkDestroyFence(pVkDevice->GetDevice(), m_acquireFence, nullptr);

		for (VkSemaphore semaphore : m_presentSemaphores)
			vkDestroySemaphore(pVkDevice->GetDevice(), semaphore, nullptr);
	}

	b8 VulkanSwapchain::AcquireNextImage()
	{
		VkDevice device = static_cast<VulkanDevice*>(m_ownerDevice)->GetDevice();

		VkAcquireNextImageInfoKHR acquireInfo{};
		acquireInfo.sType = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
		acquireInfo.fence = m_acquireFence;
		acquireInfo.swapchain = m_swapchain;
		acquireInfo.timeout = UINT64_MAX;
		acquireInfo.deviceMask = 1;

		VkResult result = vkAcquireNextImage2KHR(static_cast<VulkanDevice*>(m_ownerDevice)->GetDevice(), &acquireInfo, &m_imageIndex);
		if (result != VK_SUCCESS)
		{
			if (result == VK_SUBOPTIMAL_KHR)
				Terminal::Warn(StringHelper::GetTypeName(this), "VK_SUBOPTIMAL_KHR, image index: {}", m_imageIndex);
				
			Terminal::Error(StringHelper::GetTypeName(this), "vkAcquireNextImage2KHR failed: VkResult({})", (i32)result);
			return false;
		}

		vkWaitForFences(device, 1, &m_acquireFence, VK_TRUE, UINT64_MAX);
		vkResetFences(device, 1, &m_acquireFence);

		m_isAcquired = true;

		return true;
	}

	void VulkanSwapchain::Present()
	{
		auto* pVkDevice = (VulkanDevice*)m_ownerDevice;

		VkSemaphoreSubmitInfo signalInfo{};
		signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		signalInfo.semaphore = m_presentSemaphores[m_imageIndex];

		VkSubmitInfo2 submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.pSignalSemaphoreInfos = &signalInfo;
		submitInfo.signalSemaphoreInfoCount = 1;

		vkQueueSubmit2(pVkDevice->GetQueue(QueueType::Graphics), 
			1, &submitInfo, nullptr);

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &signalInfo.semaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapchain;
		presentInfo.pImageIndices = &m_imageIndex;

		VkResult result = vkQueuePresentKHR(pVkDevice->GetQueue(QueueType::Graphics), &presentInfo);
		if (result != VK_SUCCESS)
		{
			if (result == VK_SUBOPTIMAL_KHR)
				Terminal::Warn(StringHelper::GetTypeName(this), "VK_SUBOPTIMAL_KHR, image index: {}", m_imageIndex);
				
			Terminal::Error(StringHelper::GetTypeName(this), "vkAcquireNextImage2KHR failed: VkResult({})", (i32)result);
		}
	}

	void VulkanSwapchain::Resize(u32 width, u32 height)
	{
		if (width == 0 || height == 0 || (width == m_width && height == m_height))
			return;

		VkSurfaceCapabilitiesKHR surfaceCapabilities{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(static_cast<VulkanDevice *>(m_ownerDevice)->GetPhysicalDevice(), m_surface, &surfaceCapabilities);
		
		m_width = std::clamp(width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
		m_height = std::clamp(height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);

		ReleaseImages();
		ReCreateSwapchain();
		CreateImages();
	}

	void VulkanSwapchain::SetDebugName(const c8* pName)
	{
		if (!m_surface || !m_swapchain || !pName)
			return;

        VkDebugUtilsObjectNameInfoEXT nameInfo {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_SWAPCHAIN_KHR,
            .objectHandle = (u64)m_swapchain,
            .pObjectName = pName
        };

        if (static_cast<VulkanDevice *>(m_ownerDevice)->GetDesc().enableDebug)
            vkSetDebugUtilsObjectNameEXT(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), &nameInfo);

		nameInfo.objectType = VK_OBJECT_TYPE_SURFACE_KHR;
		nameInfo.objectHandle = (u64)m_surface;

        if (static_cast<VulkanDevice *>(m_ownerDevice)->GetDesc().enableDebug)
            vkSetDebugUtilsObjectNameEXT(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), &nameInfo);

		c8 buffer[128];
		for (u32 i = 0; i < m_imageCount; i++)
		{
			std::snprintf(buffer, sizeof(buffer), "%s[%u]", pName, i);
			m_images[i]->SetDebugName(buffer);
		}
	}

	b8 VulkanSwapchain::CreateImages()
	{
		VkResult result = vkGetSwapchainImagesKHR(static_cast<VulkanDevice*>(m_ownerDevice)->GetDevice(), m_swapchain, &m_imageCount, nullptr);
		if (result != VK_SUCCESS)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "vkGetSwapchainImagesKHR failed: VkResult({})", (i32)result);
			return false;
		}

		List<VkImage> swapchainImages(m_imageCount);
		result = vkGetSwapchainImagesKHR(static_cast<VulkanDevice*>(m_ownerDevice)->GetDevice(), m_swapchain, &m_imageCount, swapchainImages.GetData());
		
		if (result != VK_SUCCESS)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "vkGetSwapchainImagesKHR failed: VkResult({})", (i32)result);
			return false;
		}

		for (u32 i = 0; i < m_imageCount; i++)
		{
			UniquePtr<VulkanTexture> pImage = MakeUnique<VulkanTexture>();

			pImage->m_ownerDevice = m_ownerDevice;
			pImage->m_type = TextureType::Texture2D;
			pImage->m_format = m_format;
			pImage->m_usage = TextureUsage::RenderTarget;
			pImage->m_width = m_width;
			pImage->m_height = m_height;
			pImage->m_allocation = VK_NULL_HANDLE;
			pImage->m_ownerSwapchain = this;

			pImage->m_image = swapchainImages[i];

			TextureViewDesc viewDesc = {};
			viewDesc.pTexture = pImage.Get();
			viewDesc.type = TextureViewType::Texture2D;

			UniquePtr<TextureView> pView = m_ownerDevice->CreateRHITextureView(viewDesc);
			if (!pView)
				return false;

			m_images[i] = std::move(pImage);
			m_views[i] = std::move(pView);
		}

		return true;
	}

	void VulkanSwapchain::ReleaseImages()
	{
		// VkDevice device = static_cast<VulkanDevice*>(m_ownerDevice)->GetDevice();
		// vkWaitForFences(device, 
		// 	m_imagePresentationFinishedFences.GetCount(), m_imagePresentationFinishedFences.GetData(), VK_TRUE, UINT64_MAX);
		// vkResetFences(device, m_imagePresentationFinishedFences.GetCount(), m_imagePresentationFinishedFences.GetData());

		// VkReleaseSwapchainImagesInfoKHR releaseSwapchainInfo{};
		// releaseSwapchainInfo.swapchain = m_swapchain;
		// releaseSwapchainInfo.pImageIndices = &m_imageIndex;
		// releaseSwapchainInfo.imageIndexCount = 1;

		// // releases acquired image 
		// if (m_isAcquired)
		// 	vkReleaseSwapchainImagesKHR(device, &releaseSwapchainInfo);

		for (u32 i = 0; i < kMaxSwapchainImages; i++)
		{
			m_views[i] = nullptr;
			m_images[i] = nullptr;
		}
	}

	void VulkanSwapchain::ReCreateSwapchain()
	{
		VkSwapchainCreateInfoKHR swapchainCreateInfo{};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.oldSwapchain = m_swapchain;
		swapchainCreateInfo.surface = m_surface;
		swapchainCreateInfo.minImageCount = m_imageCount;
		swapchainCreateInfo.imageFormat = Helpers::ToFormat(m_format);
		swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapchainCreateInfo.imageExtent = { m_width, m_height };
		swapchainCreateInfo.imageArrayLayers = 1;
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchainCreateInfo.presentMode = m_vkPresentMode;

		vkCreateSwapchainKHR(static_cast<VulkanDevice*>(m_ownerDevice)->GetDevice(), &swapchainCreateInfo, nullptr, &m_swapchain);
	}
}
