#pragma once

#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorSet.h>

#include <vulkan/vulkan.h >

namespace Horizon
{
	class GfxVkDescriptorSet : public GfxDescriptorSet
	{
	public:
		GfxVkDescriptorSet(const GfxDescriptorSetDesc& desc, GfxDevice* pDevice);
		~GfxVkDescriptorSet() override final;

		void* DescSet() const override final;
		void Update(const GfxSetUpdateDesc& update) const override final;

	private:
		VkDescriptorSet m_set;
	};
}
