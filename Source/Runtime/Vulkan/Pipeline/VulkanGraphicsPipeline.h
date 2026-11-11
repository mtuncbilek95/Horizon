#pragma once

#include <Runtime/RHI/Pipeline/GraphicsPipeline.h>

#include <volk/volk.h>

namespace Horizon::RHI
{
	class VulkanGraphicsPipeline final : public GraphicsPipeline
	{
		friend class VulkanDevice;
	public:
		~VulkanGraphicsPipeline() final;

		void SetDebugName(const c8* pName) final;

		VkPipeline Handle() const { return m_pipeline; }

	private:
		VkPipeline m_pipeline = VK_NULL_HANDLE;
	};
}
