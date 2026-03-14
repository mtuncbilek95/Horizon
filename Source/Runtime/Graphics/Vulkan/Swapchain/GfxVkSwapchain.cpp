#include "GfxVkSwapchain.h"

#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>
#include <Runtime/Graphics/Vulkan/Image/GfxVkImage.h>
#include <Runtime/Graphics/Vulkan/Image/GfxVkImageView.h>
#include <Runtime/Graphics/Vulkan/Queue/GfxVkQueue.h>
#include <Runtime/Graphics/Vulkan/Sync/GfxVkFence.h>
#include <Runtime/Graphics/Vulkan/Sync/GfxVkSemaphore.h>
#include <Runtime/Graphics/Vulkan/Util/VkImageUtils.h>
#include <Runtime/Graphics/Vulkan/Util/VkMemoryUtils.h>

#include <vector>

namespace Horizon
{
	GfxVkSwapchain::GfxVkSwapchain(const GfxSwapchainDesc& desc, GfxDevice* pDevice) : GfxSwapchain(desc, pDevice), 
		m_swapchain(VK_NULL_HANDLE)
	{
		VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = desc.graphicsQueue->FamilyIndex();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VDebug::VkAssert(vkCreateCommandPool(VkDevice(Root()->Device()), &poolInfo, nullptr, &m_barrierPool), "GfxVkSwapchain");

		VkCommandBufferAllocateInfo bufferInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		bufferInfo.commandBufferCount = 1;
		bufferInfo.commandPool = m_barrierPool;
		bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		VDebug::VkAssert(vkAllocateCommandBuffers(VkDevice(Root()->Device()), &bufferInfo, &m_barrierBuffer), "GfxVkSwapchain");

		VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
		VDebug::VkAssert(vkCreateFence(VkDevice(Root()->Device()), &fenceInfo, nullptr, &m_barrierFence), "GfxVkSwapchain");

#if defined(HORIZON_WINDOWS)
		VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
		surfaceInfo.hinstance = GetModuleHandle(nullptr);
		surfaceInfo.hwnd = (HWND)desc.windowHandler;
		VDebug::VkAssert(vkCreateWin32SurfaceKHR(VkInstance(Root()->Instance()), &surfaceInfo, nullptr, &m_surface), "GfxVkSwapchain");
#endif

		Resize(desc.imageSize);
	}

	GfxVkSwapchain::~GfxVkSwapchain()
	{
		if (m_barrierFence != VK_NULL_HANDLE)
		{
			vkDestroyFence(VkDevice(Root()->Device()), m_barrierFence, nullptr);
			m_barrierFence = VK_NULL_HANDLE;
		}

		if (m_barrierBuffer != VK_NULL_HANDLE)
		{
			vkFreeCommandBuffers(VkDevice(Root()->Device()), m_barrierPool, 1, &m_barrierBuffer);
			m_barrierBuffer = VK_NULL_HANDLE;
		}

		if (m_barrierPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(VkDevice(Root()->Device()), m_barrierPool, nullptr);
			m_barrierPool = VK_NULL_HANDLE;
		}

		if (m_swapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(VkDevice(Root()->Device()), m_swapchain, nullptr);
			m_swapchain = VK_NULL_HANDLE;
		}

		if (m_surface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(VkInstance(Root()->Instance()), m_surface, nullptr);
			m_surface = VK_NULL_HANDLE;
		}
	}

	u32 GfxVkSwapchain::AcquireNextImage(GfxSemaphore* signal, GfxFence* fence)
	{
		VDebug::VkAssert(vkAcquireNextImageKHR(VkDevice(Root()->Device()), m_swapchain,
			UINT64_MAX, signal ? VkSemaphore(signal->Semaphore()) : VK_NULL_HANDLE, fence ? VkFence(fence->Fence()) : VK_NULL_HANDLE, &m_requestedIndex), "GfxVkSwapchain");

		return m_requestedIndex;
	}

	void GfxVkSwapchain::Present(const std::vector<GfxSemaphore*>& waits) const
	{
		std::vector<VkSemaphore> sems(waits.size());

		for (u32 i = 0; i < sems.size(); i++)
			sems[i] = VkSemaphore(waits[i]->Semaphore());

		VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
		presentInfo.waitSemaphoreCount = sems.size();
		presentInfo.pWaitSemaphores = sems.data();
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapchain;
		presentInfo.pImageIndices = &m_requestedIndex;

		VDebug::VkAssert(vkQueuePresentKHR(VkQueue(GraphicsQueue()->Queue()), &presentInfo), "GfxVkSwapchain");
	}

	void GfxVkSwapchain::Resize(const Math::Vec2u& newSize)
	{
		if (newSize.x == 0 || newSize.y == 0)
			return;

		GfxSwapchain::Resize(newSize);

		if (m_swapchain != VK_NULL_HANDLE)
			vkDestroySwapchainKHR(VkDevice(Root()->Device()), m_swapchain, nullptr);

		VkSurfaceCapabilitiesKHR surfaceCap = {};
		VDebug::VkAssert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkPhysicalDevice(Root()->Adapter()), m_surface, &surfaceCap), "GfxVkSwapchain");

		u32 formatCount = 0;
		VDebug::VkAssert(vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice(Root()->Adapter()), m_surface, &formatCount, nullptr), "GfxVkSwapchain");
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		VDebug::VkAssert(vkGetPhysicalDeviceSurfaceFormatsKHR(VkPhysicalDevice(Root()->Adapter()), m_surface, &formatCount, formats.data()), "GfxVkSwapchain");

		if (surfaceCap.maxImageExtent.width > 0 && surfaceCap.maxImageExtent.height > 0)
		{
			if (newSize.x > surfaceCap.maxImageExtent.width || newSize.y > surfaceCap.maxImageExtent.width)
				SetSize({ surfaceCap.maxImageExtent.width, surfaceCap.maxImageExtent.height });
			else
				SetSize(newSize);
		}

		u32 presentCount = 0;
		VDebug::VkAssert(vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice(Root()->Adapter()), m_surface, &presentCount, nullptr), "GfxVkSwapchain");
		std::vector<VkPresentModeKHR> presentModes(presentCount);
		VDebug::VkAssert(vkGetPhysicalDeviceSurfacePresentModesKHR(VkPhysicalDevice(Root()->Adapter()), m_surface, &presentCount, presentModes.data()), "GfxVkSwapchain");

		VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
		createInfo.surface = m_surface;
		createInfo.minImageCount = static_cast<u32>(ImageCount());
		createInfo.imageFormat = VkImageUtils::GetVkImgFormat(ImageFormat());
		createInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		createInfo.imageExtent = { ImageSize().x, ImageSize().y};
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.preTransform = surfaceCap.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = VkImageUtils::GetVkPresentMode(PresentMode());
		createInfo.clipped = VK_FALSE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;
		VDebug::VkAssert(vkCreateSwapchainKHR(VkDevice(Root()->Device()), &createInfo, nullptr, &m_swapchain), "GfxVkSwapchain");

		u32 imageCount;
		VDebug::VkAssert(vkGetSwapchainImagesKHR(VkDevice(Root()->Device()), m_swapchain, &imageCount, nullptr), "GfxVkSwapchain");

		std::vector<VkImage> images(imageCount);
		VDebug::VkAssert(vkGetSwapchainImagesKHR(VkDevice(Root()->Device()), m_swapchain, &imageCount, images.data()), "GfxVkSwapchain");

		for (u32 i = 0; i < imageCount; ++i)
		{
			GfxImageDesc imageProps = GfxImageDesc().setType(ImageType::Image2D)
				.setSize({ ImageSize().x, ImageSize().y, 1 })
				.setMipLevels(1)
				.setArrayLayers(1)
				.setFormat(ImageFormat())
				.setUsage(ImageUsage::ColorAttachment | ImageUsage::TransferDst);
			AddImage(std::make_shared<GfxVkImage>(imageProps, Root(), static_cast<void*>(images.at(i))));
			
			GfxViewDesc viewProps = GfxViewDesc();
			AddView(Image(i)->CreateView(viewProps));

			VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			VDebug::VkAssert(vkResetCommandBuffer(m_barrierBuffer, 0), "GfxVkSwapchain");
			VDebug::VkAssert(vkBeginCommandBuffer(m_barrierBuffer, &beginInfo), "GfxVkSwapchain");

			VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcAccessMask = VK_ACCESS_NONE;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = VkImage(Image(i)->Image());
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = 0;

			vkCmdPipelineBarrier(m_barrierBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			VDebug::VkAssert(vkEndCommandBuffer(m_barrierBuffer), "GfxVkSwapchain");

			VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &m_barrierBuffer;

			VDebug::VkAssert(vkQueueSubmit(VkQueue(GraphicsQueue()->Queue()), 1, &submitInfo, m_barrierFence), "GfxVkSwapchain");
			VDebug::VkAssert(vkQueueWaitIdle(VkQueue(GraphicsQueue()->Queue())), "GfxVkSwapchain");

			vkWaitForFences(VkDevice(Root()->Device()), 1, &m_barrierFence, VK_TRUE, u64_max);
			vkResetFences(VkDevice(Root()->Device()), 1, &m_barrierFence);
		}
	}
}
