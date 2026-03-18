#include "InputDispatcher.h"

namespace Horizon
{
    InputMessage InputDispatcher::BuildBase(InputType type) const
    {
        InputMessage msg;
        msg.type = type;
        msg.modifiers = m_modifiers;
        msg.mouseX = m_mouseX;
        msg.mouseY = m_mouseY;
        return msg;
    }

    void InputDispatcher::OnKeyPressed(InputCallbackFn callback)
    {
        m_keyCallbacks.push_back(std::move(callback));
    }

    void InputDispatcher::OnMousePressed(InputCallbackFn callback)
    {
        m_mouseCallbacks.push_back(std::move(callback));
    }

    void InputDispatcher::DispatchKey(KeyCode key, i32 scancode, InputAction action, InputModifiers modifiers)
    {
        m_modifiers = modifiers;

        InputMessage msg = BuildBase(InputType::Key);
        msg.key = key;
        msg.scancode = scancode;
        msg.keyAction = action;

        for (const auto& cb : m_keyCallbacks)
            cb(msg);
    }

    void InputDispatcher::DispatchMouseButton(MouseButton button, InputAction action, InputModifiers modifiers)
    {
        m_modifiers = modifiers;

        InputMessage msg = BuildBase(InputType::MouseButton);
        msg.button = button;
        msg.mouseAction = action;

        for (const auto& cb : m_mouseCallbacks)
            cb(msg);
    }

    void InputDispatcher::DispatchMouseMove(f64 x, f64 y)
    {
        m_mouseX = x;
        m_mouseY = y;

        InputMessage msg = BuildBase(InputType::MouseMove);

        for (const auto& cb : m_mouseCallbacks)
            cb(msg);
    }

    void InputDispatcher::DispatchMouseScroll(f64 xOffset, f64 yOffset)
    {
        InputMessage msg = BuildBase(InputType::MouseScroll);
        msg.scrollX = xOffset;
        msg.scrollY = yOffset;

        for (const auto& cb : m_mouseCallbacks)
            cb(msg);
    }

    void InputDispatcher::DispatchChar(u32 codepoint)
    {
        InputMessage msg = BuildBase(InputType::Char);
        msg.codepoint = codepoint;

        for (const auto& cb : m_keyCallbacks)
            cb(msg);
    }

    void InputDispatcher::DispatchResize(i32 width, i32 height)
    {
        InputMessage msg = BuildBase(InputType::Resize);
        msg.resizeWidth = width;
        msg.resizeHeight = height;

        for (const auto& cb : m_mouseCallbacks)
            cb(msg);
    }
}
