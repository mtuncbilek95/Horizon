#pragma once

#include <Runtime/RHI/Fence/Fence.h>

#include <volk/volk.h>

namespace Horizon::RHI
{
	class VulkanFence final : public Fence
	{
		friend class VulkanDevice;
	public:
		~VulkanFence() final;

		u64 GetCompletedValue() final;
		void WaitCPU(u64 value) final;

		void SetDebugName(const c8* pName) final;

		VkSemaphore Handle() const { return m_semaphore; }

	private:
		u64 Advance() { return ++m_value; }

	private:
		VkSemaphore m_semaphore = VK_NULL_HANDLE;
	};
}
