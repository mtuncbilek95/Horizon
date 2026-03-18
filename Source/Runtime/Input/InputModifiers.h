#pragma once

namespace Horizon
{
    enum class InputModifiers : i32
    {
        None = 0x0000,
        Shift = 0x0001,
        Control = 0x0002,
        Alt = 0x0004,
        Super = 0x0008,
        CapsLock = 0x0010,
        NumLock  = 0x0020,
    };
}
