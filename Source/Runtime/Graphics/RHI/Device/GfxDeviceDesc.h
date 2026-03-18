#pragma once

namespace Horizon
{
    struct GfxDeviceDesc
    {
        u32 graphicsQueueCount;
        u32 computeQueueCount;
        u32 transferQueueCount;

        GfxDeviceDesc& setGraphicsQueueCount(u32 count) { graphicsQueueCount = count; return *this; }
        GfxDeviceDesc& setComputeQueueCount(u32 count) { computeQueueCount = count; return *this; }
        GfxDeviceDesc& setTransferQueueCount(u32 count) { transferQueueCount = count; return *this; }
    };
}
