#pragma once

#include <Runtime/Graphics/RHI/Util/DescriptorFormat.h>

#include <Runtime/Graphics/Vulkan/VkIncludes.h>

namespace Horizon
{
    struct VkDescriptorUtils
    {
        static VkBufferUsageFlags GetVkDescBufferUsage(DescriptorBufferType type);
        static VkDescriptorType GetVkDescType(DescriptorType type);
        static VkDescriptorPoolCreateFlags GetVkDescPoolFlags(DescriptorPoolFlags flags);
        static VkDescriptorSetLayoutCreateFlags GetVkDescLayoutFlags(DescriptorLayoutFlags flags);
    };
}
