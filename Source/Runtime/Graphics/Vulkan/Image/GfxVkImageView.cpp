#include "GfxVkImageView.h"

#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>
#include <Runtime/Graphics/Vulkan/Image/GfxVkImage.h>
#include <Runtime/Graphics/Vulkan/Util/VkImageUtils.h>
#include <Runtime/Graphics/Vulkan/Util/VkMemoryUtils.h>

namespace Horizon
{
	GfxVkImageView::GfxVkImageView(const GfxImageViewDesc& desc, GfxDevice* pDevice) : GfxImageView(desc, pDevice)
	{
		m_image = static_cast<VkImage>(desc.image->Image());

		VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		viewInfo.image = m_image;
		viewInfo.viewType = VkImageUtils::GetVkViewType(desc.viewType);
		viewInfo.format = VkImageUtils::GetVkImgFormat(desc.image->ImageFormat());

		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		viewInfo.subresourceRange.aspectMask = VkImageUtils::GetVkAspectMask(desc.aspect);
		viewInfo.subresourceRange.baseMipLevel = desc.baseMip;
		viewInfo.subresourceRange.levelCount = desc.image->MipLevels();
		viewInfo.subresourceRange.baseArrayLayer = desc.baseArray;
		viewInfo.subresourceRange.layerCount = desc.image->ArrayLayers();

		VDebug::VkAssert(vkCreateImageView(VkDevice(Root()->Device()), &viewInfo, nullptr, &m_view), "GfxVkImageView");
	}

	GfxVkImageView::~GfxVkImageView()
	{
		if (m_view != VK_NULL_HANDLE)
		{
			vkDestroyImageView(VkDevice(Root()->Device()), m_view, nullptr);
			m_view = VK_NULL_HANDLE;
		}
	}

	void* GfxVkImageView::Image() const { return static_cast<void*>(m_image);}
	void* GfxVkImageView::View() const { return static_cast<void*>(m_view); }
}
