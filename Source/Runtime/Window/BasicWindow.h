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

    struct WindowProps final
    {
        std::string windowName = "Horizon";
        Math::Vec2u windowSize = { 1920, 1080 };
        WindowMode  windowMode = WindowMode::Windowed;

        WindowProps& setWindowName(const std::string& name) { windowName = name; return *this; }
        WindowProps& setWindowSize(const Math::Vec2u& size) { windowSize = size; return *this; }
        WindowProps& setWindowMode(WindowMode mode) { windowMode = mode; return *this; }
    };

    class BasicWindow
    {
    public:
        BasicWindow(const WindowProps& desc);
        ~BasicWindow();

        void Show();
        void Hide();
        void ProcessEvents();

        void SetTitle(const std::string& title);
        void SetSize(const Math::Vec2u& size);
        void SetWindowMode(WindowMode mode);

        const std::string& GetTitle() const { return m_props.windowName; }
        const Math::Vec2u& GetSize() const { return m_props.windowSize; }
        WindowMode GetWindowMode() const { return m_props.windowMode; }

        b8 IsHidden() const { return m_hidden; }
        b8 IsActive() const;

        void* GetHandle() const { return m_windowHandle; }
        void* GetInstance() const { return m_windowInstance; }
		void* GetNativeWindow() const;

        InputDispatcher& GetInputDispatcher() { return *m_inputDispatcher; }

    private:
        WindowProps m_props;

        void* m_windowHandle   = nullptr;
        void* m_windowInstance = nullptr;

        b8 m_hidden = true;

        std::shared_ptr<InputDispatcher> m_inputDispatcher;
    };
}
