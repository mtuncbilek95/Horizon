#pragma once

namespace Horizon
{
    // Values match GLFW_MOD_* bit-flags so a direct static_cast from the GLFW callback is safe.
    // Bitwise operators (|, &, ~, HasFlag, etc.) are provided globally by BitwiseOperator.h (PCH).
    enum class InputModifiers : i32
    {
        None     = 0x0000,
        Shift    = 0x0001,
        Control  = 0x0002,
        Alt      = 0x0004,
        Super    = 0x0008,
        CapsLock = 0x0010,
        NumLock  = 0x0020,
    };
}
