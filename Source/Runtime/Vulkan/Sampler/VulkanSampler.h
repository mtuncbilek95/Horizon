#pragma once

#include <Runtime/RHI/Sampler/Sampler.h>

#include <volk/volk.h>

namespace Horizon::RHI
{
	class VulkanSampler final : public Sampler
	{
		friend class VulkanDevice;
	public:
		const VkSamplerCreateInfo& GetCreateInfo() const { return m_samplerCreateInfo; }
	private:
		VkSamplerCreateInfo m_samplerCreateInfo = {};
	};
}
