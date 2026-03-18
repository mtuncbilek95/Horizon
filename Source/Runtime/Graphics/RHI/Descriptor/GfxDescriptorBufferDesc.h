#pragma once

#include <Runtime/Graphics/RHI/Util/BufferFormat.h>
#include <Runtime/Graphics/RHI/Util/DescriptorFormat.h>
#include <Runtime/Graphics/RHI/Util/MemoryFormat.h>

namespace Horizon
{
    struct GfxDescriptorBufferDesc
    {
        usize sizeInBytes = 0;
        DescriptorType usage = DescriptorType::Storage;
        DescriptorBufferType type = DescriptorBufferType::Resource;
        MemoryUsage memUsage = MemoryUsage::AutoPreferDevice;
        MemoryAllocation allocFlags = MemoryAllocation::Mapped;

        GfxDescriptorBufferDesc& setSize(usize size) { sizeInBytes = size; return *this; }
        GfxDescriptorBufferDesc& setUsage(DescriptorType u) { usage = u; return *this; }
        GfxDescriptorBufferDesc& setType(DescriptorBufferType t) { type = t; return *this; }
        GfxDescriptorBufferDesc& setMemoryUsage(MemoryUsage mem) { memUsage = mem; return *this; }
        GfxDescriptorBufferDesc& setAllocationFlags(MemoryAllocation flags) { allocFlags = flags; return *this; }
    };
}