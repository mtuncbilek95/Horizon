#pragma once

namespace Horizon
{
    struct GfxFenceDesc
    {
        b8 signalled = false;

        GfxFenceDesc& setSignal(b8 val) { signalled = val; return *this; }
    };
}
