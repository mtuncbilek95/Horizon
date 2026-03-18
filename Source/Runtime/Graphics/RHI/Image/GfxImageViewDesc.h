#pragma once

#include <Runtime/Graphics/RHI/Util/ImageFormat.h>
#include <Runtime/Graphics/RHI/Util/MemoryFormat.h>

namespace Horizon
{
    class GfxImage;

    struct GfxImageViewDesc
    {
        GfxImage* image = nullptr;
        ImageViewType viewType = ImageViewType::View2D;
        ImageAspect aspect = ImageAspect::Color;
        u32 baseMip = 0;
        u32 baseArray = 0;

        GfxImageViewDesc& setImage(GfxImage* img) { image = img; return *this; }
        GfxImageViewDesc& setViewType(ImageViewType type) { viewType = type; return *this; }
        GfxImageViewDesc& setAspect(ImageAspect asp) { aspect = asp; return *this; }
        GfxImageViewDesc& setBaseMip(u32 mip) { baseMip = mip; return *this; }
        GfxImageViewDesc& setBaseArray(u32 array) { baseArray = array; return *this; }
    };
}
