#include "VulkanCommandPool.h"
#include "Runtime/Definitions/PrimitiveDefinitions.h"

#include <Runtime/Vulkan/Device/VulkanDevice.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

#include <cstring>

namespace Horizon::RHI
{
	VulkanCommandPool::~VulkanCommandPool()
	{
		if (m_cmdPool)
			vkDestroyCommandPool(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), m_cmdPool, nullptr);
	}

	void VulkanCommandPool::Reset()
	{
		VkResult result = vkResetCommandPool(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), m_cmdPool, 0);
		if (result != VK_SUCCESS)
			Terminal::Error(StringHelper::GetTypeName(this), "Command pool reset failed, a list may still be recording or in flight VkResult({})", (i32)result);
	}

	void VulkanCommandPool::SetDebugName(const c8* pName)
	{
		if (!m_cmdPool || !pName)
			return;

        const VkDebugUtilsObjectNameInfoEXT nameInfo {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_COMMAND_POOL,
            .objectHandle = (u64)m_cmdPool,
            .pObjectName = pName
        };

        if (static_cast<VulkanDevice *>(m_ownerDevice)->GetDesc().enableDebug)
            vkSetDebugUtilsObjectNameEXT(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), &nameInfo);
	}
}
