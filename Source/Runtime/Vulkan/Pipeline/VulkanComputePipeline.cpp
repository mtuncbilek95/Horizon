#include "VulkanComputePipeline.h"

#include <Runtime/Vulkan/Device/VulkanDevice.h>

namespace Horizon::RHI
{
	VulkanComputePipeline::~VulkanComputePipeline()
	{
		if (m_pipeline)
			vkDestroyPipeline(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), m_pipeline, nullptr);
	}

	void VulkanComputePipeline::SetDebugName(const c8* pName)
	{
		if (!m_pipeline || !pName)
			return;

        const VkDebugUtilsObjectNameInfoEXT nameInfo {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            .objectType = VK_OBJECT_TYPE_PIPELINE,
            .objectHandle = (u64)m_pipeline,
            .pObjectName = pName
        };

        if (static_cast<VulkanDevice *>(m_ownerDevice)->GetDesc().enableDebug)
            vkSetDebugUtilsObjectNameEXT(static_cast<VulkanDevice *>(m_ownerDevice)->GetDevice(), &nameInfo);
	}
}
