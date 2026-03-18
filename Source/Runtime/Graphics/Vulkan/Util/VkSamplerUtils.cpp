#include "VkSamplerUtils.h"

namespace Horizon
{
    VkFilter VkSamplerUtils::GetVkFilter(SamplerFilter filter)
    {
        switch (filter)
        {
        case SamplerFilter::Nearest: 
            return VK_FILTER_NEAREST;
        case SamplerFilter::Linear:  
            return VK_FILTER_LINEAR;
        case SamplerFilter::Cubic:   
            return VK_FILTER_CUBIC_EXT;
        default:                     
            return VK_FILTER_NEAREST;
        }
    }

    VkSamplerMipmapMode VkSamplerUtils::GetVkMipMap(SamplerMipMap mode)
    {
        switch (mode)
        {
        case SamplerMipMap::Nearest: 
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case SamplerMipMap::Linear:  
            return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        default:                     
            return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        }
    }

    VkSamplerAddressMode VkSamplerUtils::GetVkSamplerAddress(SamplerAddress address)
    {
        switch (address)
        {
        case SamplerAddress::Repeat:            
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case SamplerAddress::MirroredRepeat:    
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case SamplerAddress::ClampToEdge:       
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case SamplerAddress::ClampToBorder:     
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case SamplerAddress::MirrorClampToEdge: 
            return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        default:                                
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        }
    }

    VkBorderColor VkSamplerUtils::GetVkColor(BorderColor color)
    {
        switch (color)
        {
        case BorderColor::TransparentBlack_Float: 
            return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        case BorderColor::TransparentBlack_Int:   
            return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
        case BorderColor::OpaqueBlack_Float:      
            return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        case BorderColor::OpaqueBlack_Int:        
            return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        default:                                   
            return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        }
    }

}
