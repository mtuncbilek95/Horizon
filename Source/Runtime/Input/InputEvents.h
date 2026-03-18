#pragma once

#include <Runtime/Input/InputAction.h>
#include <Runtime/Input/InputKeyCode.h>
#include <Runtime/Input/InputModifiers.h>
#include <Runtime/Input/InputMouseButton.h>

namespace Horizon
{
    enum class InputType
    {
        Key,
        MouseButton,
        MouseMove,
        MouseScroll,
        Char,
        Resize,
    };

    struct InputMessage
    {
        InputType type = InputType::Key;

        InputModifiers modifiers = InputModifiers::None;
        f64 mouseX = 0.0;
        f64 mouseY = 0.0;

        KeyCode key = KeyCode::Unknown;
        InputAction keyAction = InputAction::Release;
        i32 scancode = 0;
        u32 codepoint = 0;

        MouseButton button = MouseButton::Left;
        InputAction mouseAction = InputAction::Release;

        f64 scrollX = 0.0;
        f64 scrollY = 0.0;

        i32 resizeWidth = 0;
        i32 resizeHeight = 0;
    };
}
