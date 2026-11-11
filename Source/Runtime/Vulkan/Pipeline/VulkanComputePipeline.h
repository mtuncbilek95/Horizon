#pragma once

#include <Runtime/RHI/Pipeline/ComputePipeline.h>

#include <volk/volk.h>

namespace Horizon::RHI
{
	class VulkanComputePipeline final : public ComputePipeline
	{
		friend class VulkanDevice;
	public:
		~VulkanComputePipeline() final;

		void SetDebugName(const c8* pName) final;

		VkPipeline Handle() const { return m_pipeline; }

	private:
		VkPipeline m_pipeline = VK_NULL_HANDLE;
	};
}
