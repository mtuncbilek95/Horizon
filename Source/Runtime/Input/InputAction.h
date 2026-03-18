#pragma once

namespace Horizon
{
    // Values match GLFW_RELEASE / GLFW_PRESS / GLFW_REPEAT so a direct static_cast is safe.
    enum class InputAction : i32
    {
        Release = 0,
        Press = 1,
        Repeat = 2,
    };
}
