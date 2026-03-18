#pragma once

#include <Runtime/Graphics/RHI/Util/SamplerFormat.h>

#include <Runtime/Graphics/Vulkan/VkIncludes.h>

namespace Horizon
{
    struct VkSamplerUtils
    {
        static VkFilter GetVkFilter(SamplerFilter filter);
        static VkSamplerMipmapMode GetVkMipMap(SamplerMipMap mode);
        static VkSamplerAddressMode GetVkSamplerAddress(SamplerAddress address);
        static VkBorderColor GetVkColor(BorderColor color);
    };
}
