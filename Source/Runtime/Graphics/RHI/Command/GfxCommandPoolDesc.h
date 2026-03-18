#pragma once

#include <Runtime/Graphics/RHI/Util/CommandFormat.h>

namespace Horizon
{
    class GfxQueue;

    struct GfxCommandPoolDesc
    {
        GfxQueue* queue = nullptr;
        CommandPoolFlags flags;

        GfxCommandPoolDesc& setQueue(GfxQueue* q) { queue = q; return *this; }
        GfxCommandPoolDesc& setFlags(CommandPoolFlags flag) { flags = flag; return *this; }
    };
}
