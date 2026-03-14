#pragma once

#include <Runtime/Graphics/RHI/Util/DescriptorFormat.h>

#include <vector>

namespace Horizon
{
    struct PoolSize
    {
        DescriptorType type;
        u32 count;

        PoolSize& setType(DescriptorType t) { type = t; return *this; }
        PoolSize& setCount(u32 c) { count = c; return *this; }
    };

    struct GfxDescriptorPoolDesc
    {
        std::vector<PoolSize> poolSize;
        u32 maxSets;
        DescriptorPoolFlags flags;

        GfxDescriptorPoolDesc& addPoolSize(const PoolSize& p) { poolSize.push_back(p); return *this; }
        GfxDescriptorPoolDesc& setMaxSets(u32 sets) { maxSets = sets; return *this; }
        GfxDescriptorPoolDesc& setFlags(DescriptorPoolFlags f) { flags = f; return *this; }
    };
 }
