#pragma once

#include <Runtime/Graphics/RHI/Image/GfxImageDesc.h>
#include <Runtime/Graphics/RHI/Object/GfxObject.h>

#include <memory>

namespace Horizon
{
    class GfxImageView;

    class GfxImage : public GfxObject
    {
    public:
        GfxImage(const GfxImageDesc& desc, GfxDevice* pDevice);
        GfxImage(const GfxImageDesc& desc, GfxDevice* pDevice, void* apiImage);
        virtual ~GfxImage() override = default;

        virtual void* Image() const = 0;
        virtual std::shared_ptr<GfxImageView> CreateView(const GfxViewDesc& desc) = 0;

        const Math::Vec3u& ImgSize() const { return m_desc.imgSize; }
        u32 MipLevels() const { return m_desc.mipLevels; }
        u32 ArrayLayers() const { return m_desc.arrayLayers; }
        ImageType ImageType() const { return m_desc.type; }
        ImageFormat ImageFormat() const { return m_desc.format; }
        ImageUsage ImageUsage() const { return m_desc.usage; }

        b8 IsSwapchain() const { return m_isSwapchain; }

    private:
        GfxImageDesc m_desc;
        b8 m_isSwapchain;
    };
}
