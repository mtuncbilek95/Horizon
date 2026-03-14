#include "GfxVkDescriptorPool.h"

#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>
#include <Runtime/Graphics/Vulkan/Util/VkDescriptorUtils.h>

#include <vector>

namespace Horizon
{
	GfxVkDescriptorPool::GfxVkDescriptorPool(const GfxDescriptorPoolDesc& desc, GfxDevice* pDevice) : GfxDescriptorPool(desc, pDevice)
	{
		std::vector<VkDescriptorPoolSize> poolSize(desc.poolSize.size());
		for (usize i = 0; i < desc.poolSize.size(); i++)
			poolSize[i] = { VkDescriptorUtils::GetVkDescType(desc.poolSize[i].type), desc.poolSize[i].count };

		VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
		poolInfo.maxSets = desc.maxSets;
		poolInfo.flags = VkDescriptorUtils::GetVkDescPoolFlags(desc.flags);
		poolInfo.poolSizeCount = static_cast<u32>(desc.poolSize.size());
		poolInfo.pPoolSizes = poolSize.data();

		VDebug::VkAssert(vkCreateDescriptorPool(VkDevice(Root()->Device()), &poolInfo, nullptr, &m_pool), "GfxVkDescriptorPool");
	}

	GfxVkDescriptorPool::~GfxVkDescriptorPool()
	{
		if (m_pool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(VkDevice(Root()->Device()), m_pool, nullptr);
			m_pool = VK_NULL_HANDLE;
		}
	}

	void* GfxVkDescriptorPool::DescPool() const { return static_cast<void*>(m_pool); }
}
