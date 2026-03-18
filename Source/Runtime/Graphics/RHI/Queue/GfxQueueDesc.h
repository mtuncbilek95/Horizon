#pragma once

namespace Horizon
{
    enum class QueueType
    {
        Graphics,
        Compute,
        Transfer,
        Sparse
    };

    struct GfxQueueDesc
    {
        QueueType type;
        u32 familyIndex;

        GfxQueueDesc& setType(QueueType t) { type = t; return *this; }
        GfxQueueDesc& setIndex(u32 index) { familyIndex = index; return *this; }
    };
}
