#pragma once

namespace Horizon
{
    // Values match GLFW_MOUSE_BUTTON_* so a direct static_cast from the GLFW callback is safe.
    enum class MouseButton : i32
    {
        Left    = 0,
        Right   = 1,
        Middle  = 2,
        Button4 = 3,
        Button5 = 4,
        Button6 = 5,
        Button7 = 6,
        Button8 = 7,
    };
}
