#pragma once

#include <Runtime/Graphics/RHI/Util/ImageFormat.h>
#include <Runtime/Graphics/RHI/Util/PresentMode.h>

#include <vulkan/vulkan.h>

namespace Horizon
{
	struct VkImageUtils
	{
		static VkImageType GetVkImgType(ImageType type);
		static VkFormat GetVkImgFormat(ImageFormat format);
		static VkImageUsageFlags GetVkImgUsage(ImageUsage usage);
		static VkImageAspectFlags GetVkAspectMask(ImageAspect aspect);
		static VkImageViewType GetVkViewType(ImageViewType type);
		static VkPresentModeKHR GetVkPresentMode(PresentMode mode);
		static u32 GetVkImgSize(ImageFormat type);
	};
}
