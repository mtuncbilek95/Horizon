#pragma once

#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorLayout.h>

#include <vulkan/vulkan.h>

namespace Horizon
{
	class GfxVkDescriptorLayout : public GfxDescriptorLayout
	{
	public:
		GfxVkDescriptorLayout(const GfxDescriptorLayoutDesc& desc, GfxDevice* pDevice);
		~GfxVkDescriptorLayout() override final;

		void* DescLayout() const override final;

	private:
		VkDescriptorSetLayout m_layout;
	};
}
