#pragma once

#include <Runtime/Graphics/RHI/Util/MemoryFormat.h>

#include <Runtime/Graphics/Vulkan/VkIncludes.h>
#include <vk_mem_alloc.h>

namespace Horizon
{
    struct VmaUtils
    {
        static VmaMemoryUsage GetVmaUsage(MemoryUsage usage);
        static VmaAllocationCreateFlags GetVmaFlags(MemoryAllocation alloc);
    };
}
