#include "VulkanFence.h"
#include <Runtime/Vulkan/Device/VulkanDevice.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::RHI
{
	VulkanFence::~VulkanFence()
	{
		if (m_semaphore)
			vkDestroySemaphore(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), m_semaphore, nullptr);
	}

	u64 VulkanFence::GetCompletedValue() { 
		u64 value{};
		vkGetSemaphoreCounterValue(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), m_semaphore, &value);
		return value;
	}

	void VulkanFence::WaitCPU(u64 value)
	{
		const VkSemaphoreWaitInfo waitInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1,
			.pSemaphores = &m_semaphore,
			.pValues = &value
		};

		VkResult result = vkWaitSemaphores(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), &waitInfo, UINT64_MAX);

		if (result != VK_SUCCESS)
			Terminal::Error(StringHelper::GetTypeName(this), "vkWaitSemaphores failed, VkResult({})", (i32)result);
	}

	void VulkanFence::SetDebugName(const c8* pName)
	{
		if (!m_semaphore || !pName)
			return;

        const VkDebugUtilsObjectNameInfoEXT nameInfo {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_SEMAPHORE,
            .objectHandle = (u64)m_semaphore,
            .pObjectName = pName
        };

        if (static_cast<VulkanDevice *>(m_ownerDevice)->GetDesc().enableDebug)
            vkSetDebugUtilsObjectNameEXT(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), &nameInfo);
	}
}
