#include "VulkanTexture.h"

#include <Runtime/Vulkan/Device/VulkanDevice.h>

namespace Horizon::RHI
{
	VulkanTexture::~VulkanTexture()
	{
        if (m_allocation)
            vmaDestroyImage(static_cast<VulkanDevice *>(m_ownerDevice)->GetAllocator(), m_image, m_allocation);
	}

	void VulkanTexture::SetDebugName(const c8* pName)
	{
		if (!m_image || !pName)
			return;

        const VkDebugUtilsObjectNameInfoEXT nameInfo {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_IMAGE,
            .objectHandle = (u64)m_image,
            .pObjectName = pName
        };

        if (static_cast<VulkanDevice *>(m_ownerDevice)->GetDesc().enableDebug)
            vkSetDebugUtilsObjectNameEXT(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), &nameInfo);
    }
}