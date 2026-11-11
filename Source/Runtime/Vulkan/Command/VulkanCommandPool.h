#pragma once

#include <Runtime/RHI/Command/CommandPool.h>

#include <volk/volk.h>

namespace Horizon::RHI
{
	class VulkanCommandPool final : public CommandPool
	{
		friend class VulkanDevice;
	public:
		~VulkanCommandPool() final;

		void Reset() final;

		void SetDebugName(const c8* pName) final;

		VkCommandPool Handle() const { return m_cmdPool; }

	private:
		VkCommandPool m_cmdPool = VK_NULL_HANDLE;
	};
}
