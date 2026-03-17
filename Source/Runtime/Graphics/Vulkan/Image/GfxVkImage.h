#pragma once

#include <Runtime/Graphics/RHI/Image/GfxImage.h>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <memory>

namespace Horizon
{
	class GfxImageView;

	class GfxVkImage final : public GfxImage
	{
	public:
		GfxVkImage(const GfxImageDesc& desc, GfxDevice* pDevice);
		GfxVkImage(const GfxImageDesc& desc, GfxDevice* pDevice, void* apiImage);
		~GfxVkImage() override final;

		void* Image() const override final;
		std::shared_ptr<GfxImageView> CreateView(const GfxViewDesc& desc) override final;

	private:
		VkImage m_image;

		VmaAllocation m_allocation;
		VmaAllocationInfo m_allocationInfo;
	};
}
