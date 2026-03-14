#include "GfxVkImage.h"

#include <Runtime/Graphics/Vulkan/Debug/VDebug.h>
#include <Runtime/Graphics/Vulkan/Device/GfxVkDevice.h>
#include <Runtime/Graphics/Vulkan/Image/GfxVkImageView.h>
#include <Runtime/Graphics/Vulkan/Util/VkImageUtils.h>
#include <Runtime/Graphics/Vulkan/Util/VkMemoryUtils.h>

#include <memory>

namespace Horizon
{
    static ImageViewType CastType(ImageType type, u32 layerCount)
    {
        if (HasFlag(type, ImageType::Image2D))
        {
            if (layerCount == 6)
                return ImageViewType::ViewCube;
            else if (layerCount > 6 && layerCount % 6 == 0)
                return ImageViewType::ViewCubeArray;
        }

        switch (type)
        {
        case ImageType::Image1D:
            return layerCount > 1 ? ImageViewType::View1DArray : ImageViewType::View1D;
        case ImageType::Image2D:
            return layerCount > 1 ? ImageViewType::View2DArray : ImageViewType::View2D;
        case ImageType::Image3D:
            return layerCount > 1 ? ImageViewType::View2DArray : ImageViewType::View3D;
        default:
            Log::Terminal(LogType::Error, "Render", "You're using wrong ImageType or ImageViewType");
            break;
        }

        return ImageViewType::View2D;
    }

	GfxVkImage::GfxVkImage(const GfxImageDesc& desc, GfxDevice* pDevice) : GfxImage(desc, pDevice)
	{
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VkImageUtils::GetVkImgType(desc.type);
        imageInfo.extent = { desc.imgSize.x , desc.imgSize.y, desc.imgSize.z };
        imageInfo.mipLevels = desc.mipLevels;
        imageInfo.arrayLayers = desc.arrayLayers;
        imageInfo.format = VkImageUtils::GetVkImgFormat(desc.format);
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VkImageUtils::GetVkImgUsage(desc.usage);
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VmaUtils::GetVmaUsage(desc.memUsage);
        allocInfo.flags = VmaUtils::GetVmaFlags(desc.memFlags);

        VDebug::VkAssert(vmaCreateImage(VmaAllocator(Root()->Allocator()), &imageInfo, &allocInfo,
            &m_image, &m_allocation, &m_allocationInfo), "GfxVkImage");
	}

    GfxVkImage::GfxVkImage(const GfxImageDesc& desc, GfxDevice* pDevice, void* apiImage) : GfxImage(desc, pDevice, apiImage), 
        m_image(static_cast<VkImage>(apiImage)), m_allocation(VK_NULL_HANDLE), m_allocationInfo(VmaAllocationInfo())
    {
    }

	GfxVkImage::~GfxVkImage()
	{
        if (m_allocation && !IsSwapchain())
        {
            vmaDestroyImage(VmaAllocator(Root()->Allocator()), m_image, m_allocation);
            m_allocation = nullptr;
        }
	}

	std::shared_ptr<GfxImageView> GfxVkImage::CreateView(const GfxViewDesc& desc)
	{
        GfxImageViewDesc viewProp = GfxImageViewDesc().setImage(this)
            .setViewType(CastType(ImageType(), ArrayLayers()))
            .setAspect(desc.aspect)
            .setBaseMip(desc.baseMip)
            .setBaseArray(desc.baseArray);

        return std::make_shared<GfxVkImageView>(viewProp, Root());
	}

    void* GfxVkImage::Image() const { return static_cast<void*>(m_image); }
}
