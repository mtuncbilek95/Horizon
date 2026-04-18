#pragma once

#include <Runtime/Input/InputDispatcher.h>

#include <memory>
#include <string>

namespace Horizon
{
    enum class WindowMode
    {
        Windowed,
        Fullscreen,
        Borderless
    };

    struct WindowDesc final
    {
        std::string windowName = "Horizon";
        Math::Vec2u windowSize = { 1920, 1080 };
        WindowMode  windowMode = WindowMode::Windowed;

        WindowDesc& setWindowName(const std::string& name) { windowName = name; return *this; }
        WindowDesc& setWindowSize(const Math::Vec2u& size) { windowSize = size; return *this; }
        WindowDesc& setWindowMode(WindowMode mode) { windowMode = mode; return *this; }
    };

    class Window
    {
    public:
        Window(const WindowDesc& desc);
        ~Window();

        void Show();
        void Hide();
        void ProcessEvents();

        void SetTitle(const std::string& title);
        void SetSize(const Math::Vec2u& size);
        void SetWindowMode(WindowMode mode);

        const std::string& GetTitle() const { return m_desc.windowName; }
        const Math::Vec2u& GetSize() const { return m_desc.windowSize; }
        WindowMode GetWindowMode() const { return m_desc.windowMode; }

        b8 IsHidden() const { return m_hidden; }
        b8 IsActive() const;

        void* GetHandle() const { return m_windowHandle; }
        void* GetInstance() const { return m_windowInstance; }
        void* GetNativeWindow() const;

        InputDispatcher& GetInputDispatcher() { return *m_inputDispatcher; }

    private:
        WindowDesc m_desc;

        void* m_windowHandle   = nullptr;
        void* m_windowInstance = nullptr;

        b8 m_hidden = true;

        std::shared_ptr<InputDispatcher> m_inputDispatcher;
    };
}
