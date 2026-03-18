#include "BasicWindow.h"

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#if defined(HORIZON_LINUX)
#include <X11/Xlib-xcb.h>
#endif

#include <memory>
#include <string>

namespace Horizon
{
    GLFWwindow* gWindow = nullptr;

    static void MouseMoveCallback(GLFWwindow* window, f64 xpos, f64 ypos)
    {
        auto* dispatcher = static_cast<InputDispatcher*>(glfwGetWindowUserPointer(window));
        if (dispatcher)
            dispatcher->DispatchMouseMove(xpos, ypos);
    }

    static void MouseButtonCallback(GLFWwindow* window, i32 button, i32 action, i32 mods)
    {
        auto* dispatcher = static_cast<InputDispatcher*>(glfwGetWindowUserPointer(window));
        if (dispatcher)
            dispatcher->DispatchMouseButton(
                static_cast<MouseButton>(button),
                static_cast<InputAction>(action),
                static_cast<InputModifiers>(mods));
    }

    static void ResizeCallback(GLFWwindow* window, i32 width, i32 height)
    {
        auto* dispatcher = static_cast<InputDispatcher*>(glfwGetWindowUserPointer(window));
        if (dispatcher)
            dispatcher->DispatchResize(width, height);
    }

    static void MouseWheelScrollCallback(GLFWwindow* window, f64 xoffset, f64 yoffset)
    {
        auto* dispatcher = static_cast<InputDispatcher*>(glfwGetWindowUserPointer(window));
        if (dispatcher)
            dispatcher->DispatchMouseScroll(xoffset, yoffset);
    }

    static void KeyboardCallback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 mods)
    {
        auto* dispatcher = static_cast<InputDispatcher*>(glfwGetWindowUserPointer(window));
        if (dispatcher)
            dispatcher->DispatchKey(
                static_cast<KeyCode>(key),
                scancode,
                static_cast<InputAction>(action),
                static_cast<InputModifiers>(mods));
    }

    static void CharCallback(GLFWwindow* window, u32 codepoint)
    {
        auto* dispatcher = static_cast<InputDispatcher*>(glfwGetWindowUserPointer(window));
        if (dispatcher)
            dispatcher->DispatchChar(codepoint);
    }

    BasicWindow::BasicWindow(const WindowProps& desc) : m_props(desc)
    {
        m_inputDispatcher = std::make_shared<InputDispatcher>();

        if (!glfwInit())
        {
            Log::Terminal(LogType::Fatal, "Window", "Failed to initialize GLFW");
            exit(-1);
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

        gWindow = glfwCreateWindow(m_props.windowSize.x, m_props.windowSize.y, m_props.windowName.data(), nullptr, nullptr);
        if (!gWindow)
        {
            Log::Terminal(LogType::Fatal, "Window", "Failed to create GLFW window");
            glfwTerminate();
            exit(-1);
        }

        glfwSetWindowUserPointer(gWindow, m_inputDispatcher.get());

#if defined(HORIZON_WINDOWS)
        m_windowHandle = (void*)glfwGetWin32Window(gWindow);
        m_windowInstance = nullptr;
#endif

        glfwSetCursorPosCallback(gWindow, MouseMoveCallback);
        glfwSetMouseButtonCallback(gWindow, MouseButtonCallback);
        glfwSetScrollCallback(gWindow, MouseWheelScrollCallback);
        glfwSetWindowSizeCallback(gWindow, ResizeCallback);
        glfwSetKeyCallback(gWindow, KeyboardCallback);
        glfwSetCharCallback(gWindow, CharCallback);
    }

    BasicWindow::~BasicWindow()
    {
        if (gWindow)
        {
            glfwSetWindowShouldClose(gWindow, GLFW_TRUE);
            glfwDestroyWindow(gWindow);
            glfwTerminate();
            gWindow = nullptr;
        }
    }

    void BasicWindow::Show()
    {
        if (gWindow)
            glfwShowWindow(gWindow);

        m_hidden = false;
    }

    void BasicWindow::Hide()
    {
        if (gWindow)
            glfwHideWindow(gWindow);

        m_hidden = true;
    }

    void BasicWindow::ProcessEvents()
    {
        glfwPollEvents();
    }

    void BasicWindow::SetTitle(const std::string& title)
    {
        if (gWindow)
            glfwSetWindowTitle(gWindow, title.data());
        m_props.windowName = title;
    }

    void BasicWindow::SetSize(const Math::Vec2u& size)
    {
        if (gWindow)
            glfwSetWindowSize(gWindow, size.x, size.y);

        m_props.windowSize = size;
    }

    void BasicWindow::SetWindowMode(WindowMode mode)
    {
    }

    b8 BasicWindow::IsActive() const
    {
        return !glfwWindowShouldClose(gWindow);
    }

    void* BasicWindow::GetNativeWindow() const
    {
        return gWindow;
    }
}
