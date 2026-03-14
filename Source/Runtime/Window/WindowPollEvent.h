#pragma once

namespace Horizon
{
    enum class WindowPollEvent
    {
        None,

        KeyPressed,
        KeyReleased,
        KeyChar,

        MousePressed,
        MouseReleased,
        MouseMoved,
        MouseScrolled,
        MouseDoubleClick,

        GamepadButtonPressed,
        GamepadButtonReleased,
        GamepadAxisMoved,

        WindowResized,
        WindowMoved,
        WindowClosed,
        WindowFocused,
        WindowLostFocus,

        RawInputDeviceChanged,

        DragEnter,
        DragLeave,
        DragDrop,

        AppSetFocusAs,
        MonitorChanged,
    };
}
