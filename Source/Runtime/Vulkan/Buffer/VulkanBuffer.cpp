#include "VulkanBuffer.h"

#include <Runtime/Vulkan/Device/VulkanDevice.h>

namespace Horizon::RHI
{
	VulkanBuffer::~VulkanBuffer()
	{
        if (m_buffer)
            vmaDestroyBuffer(static_cast<VulkanDevice *>(m_ownerDevice)->GetAllocator(), m_buffer, m_allocation);
	}

	void VulkanBuffer::SetDebugName(const c8* pName)
	{
        const VkDebugUtilsObjectNameInfoEXT nameInfo {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_BUFFER,
            .objectHandle = (u64)m_buffer,
            .pObjectName = pName
        };

        if (static_cast<VulkanDevice *>(m_ownerDevice)->GetDesc().enableDebug)
            vkSetDebugUtilsObjectNameEXT(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), &nameInfo);
    }
}