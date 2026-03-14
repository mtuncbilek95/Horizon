#pragma once

#include <Runtime/Input/InputEvents.h>

#include <functional>
#include <vector>

namespace Horizon
{
    class InputDispatcher
    {
    public:
        using InputCallbackFn = std::function<void(const InputMessage&)>;

        // Subscribe to keyboard events (Key + Char).
        // The received InputMessage always contains current mouse position too.
        void OnKeyPressed(InputCallbackFn callback);

        // Subscribe to pointer / window events (MouseButton + MouseMove + MouseScroll + Resize).
        // The received InputMessage always contains current modifiers too.
        void OnMousePressed(InputCallbackFn callback);

        // ── Internal dispatch — called by BasicWindow's GLFW callbacks ────────
        void DispatchKey(KeyCode key, i32 scancode, InputAction action, InputModifiers modifiers);
        void DispatchMouseButton(MouseButton button, InputAction action, InputModifiers modifiers);
        void DispatchMouseMove(f64 x, f64 y);
        void DispatchMouseScroll(f64 xOffset, f64 yOffset);
        void DispatchChar(u32 codepoint);
        void DispatchResize(i32 width, i32 height);

    private:
        InputMessage BuildBase(InputType type) const;

        // Running state — kept current so every message carries full context.
        InputModifiers m_modifiers = InputModifiers::None;
        f64            m_mouseX    = 0.0;
        f64            m_mouseY    = 0.0;

        std::vector<InputCallbackFn> m_keyCallbacks;
        std::vector<InputCallbackFn> m_mouseCallbacks;
    };
}
