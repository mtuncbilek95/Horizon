#pragma once

#include <Runtime/Graphics/RHI/Util/BufferFormat.h>
#include <Runtime/Graphics/RHI/Util/DescriptorFormat.h>

#include <Runtime/Graphics/Vulkan/VkIncludes.h>

namespace Horizon
{
    struct VkBufferUtils
    {
        static VkBufferUsageFlags GetVkBufferUsage(BufferUsage usage);
        static VkBufferUsageFlags GetVkDescBufferUsage(DescriptorBufferType type);
        static VkBufferUsageFlags2KHR GetVkDescBufferUsage2(DescriptorBufferType type);
    };
}
