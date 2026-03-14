#pragma once

#include <Runtime/Graphics/RHI/Image/GfxImageView.h>
#include <Runtime/Graphics/RHI/Image/GfxImageViewDesc.h>

#include <vulkan/vulkan.h>

namespace Horizon
{
	class GfxVkImageView : public GfxImageView
	{
	public:
		GfxVkImageView(const GfxImageViewDesc& desc, GfxDevice* pDevice);
		~GfxVkImageView() override final;

		void* Image() const override final;
		void* View() const override final;

	private:
		VkImage m_image;
		VkImageView m_view;
	};
}
