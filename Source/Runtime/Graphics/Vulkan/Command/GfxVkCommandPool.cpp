#include "GfxVkCommandPool.h"

#include <Runtime/Graphics/Vulkan/Command/GfxVkCommandBuffer.h>
#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>
#include <Runtime/Graphics/Vulkan/Queue/GfxVkQueue.h>
#include <Runtime/Graphics/Vulkan/Util/VkCommandUtils.h>

namespace Horizon
{
	GfxVkCommandPool::GfxVkCommandPool(const GfxCommandPoolDesc& desc, GfxDevice* pDevice) : GfxCommandPool(desc, pDevice)
	{
		VkCommandPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
		poolInfo.queueFamilyIndex = desc.queue->FamilyIndex();
		poolInfo.flags = VkCommandUtils::GetVkPoolFlags(desc.flags);

		VDebug::VkAssert(vkCreateCommandPool(VkDevice(Root()->Device()), &poolInfo, nullptr, &m_pool), "VCmdPool");
	}

	GfxVkCommandPool::~GfxVkCommandPool()
	{
		if (m_pool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(VkDevice(Root()->Device()), m_pool, nullptr);
			m_pool = VK_NULL_HANDLE;
		}
	}

	void* GfxVkCommandPool::Pool() const { return static_cast<void*>(m_pool); }

	std::shared_ptr<GfxCommandBuffer> GfxVkCommandPool::CreateBuffer(CommandLevel lvl)
	{
		GfxCommandBufferDesc desc = GfxCommandBufferDesc().setPool(this)
			.setLevel(lvl);

		return std::make_shared<GfxVkCommandBuffer>(desc, Root());
	}
}
