#pragma once

#include <Runtime/Graphics/RHI/Sampler/GfxSampler.h>

#include <vulkan/vulkan.h>

namespace Horizon
{
	class GfxVkSampler : public GfxSampler
	{
	public:
		GfxVkSampler(const GfxSamplerDesc& desc, GfxDevice* pDevice);
		~GfxVkSampler() override final;

		void* Sampler() const override final;

	private:
		VkSampler m_sampler;
	};
}
