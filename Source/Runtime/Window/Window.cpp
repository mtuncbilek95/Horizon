#include "Window.h"

#include <Runtime/Window/InputDispatcher.h>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Horizon
{
	GLFWwindow* gWindow = nullptr;

	static void MouseMoveCallback(GLFWwindow* window, f64 positionX, f64 positionY)
	{
		auto* dispatcher = static_cast<InputDispatcher*>(glfwGetWindowUserPointer(window));
		if (dispatcher)
			dispatcher->DispatchMouseMove(positionX, positionY);
	}

	static void MouseButtonCallback(GLFWwindow* window, i32 button, i32 action, i32 modifiers)
	{
		auto* dispatcher = static_cast<InputDispatcher*>(glfwGetWindowUserPointer(window));
		if (dispatcher)
			dispatcher->DispatchMouseButton(static_cast<MouseButton>(button),
				static_cast<InputAction>(action), static_cast<InputModifiers>(modifiers));
	}

	static void ResizeCallback(GLFWwindow* window, i32 width, i32 height)
	{
		MainWindow().SetSize({ (u32)width, (u32)height });

		auto* dispatcher = static_cast<InputDispatcher*>(glfwGetWindowUserPointer(window));
		if (dispatcher)
			dispatcher->DispatchResize(width, height);
	}

	static void MouseWheelScrollCallback(GLFWwindow* window, f64 offsetX, f64 offsetY)
	{
		auto* dispatcher = static_cast<InputDispatcher*>(glfwGetWindowUserPointer(window));
		if (dispatcher)
			dispatcher->DispatchMouseScroll(offsetX, offsetY);
	}

	static void KeyboardCallback(GLFWwindow* window, i32 key, i32 scancode, i32 action, i32 modifiers)
	{
		auto* dispatcher = static_cast<InputDispatcher*>(glfwGetWindowUserPointer(window));
		if (dispatcher)
			dispatcher->DispatchKey(static_cast<KeyCode>(key), scancode,
				static_cast<InputAction>(action), static_cast<InputModifiers>(modifiers));
	}

	static void CharCallback(GLFWwindow* window, u32 codepoint)
	{
		auto* dispatcher = static_cast<InputDispatcher*>(glfwGetWindowUserPointer(window));
		if (dispatcher)
			dispatcher->DispatchChar(codepoint);
	}

	Window::Window(const WindowDesc& desc) : m_desc(desc)
	{
		if (!glfwInit())
			exit(-1);

		const b8 borderless = (m_desc.mode == WindowMode::Borderless);

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, borderless ? GLFW_FALSE : GLFW_TRUE);

		u32 windowWidth = m_desc.size.x;
		u32 windowHeight = m_desc.size.y;

		if (borderless)
		{
			windowWidth = (u32)videoMode->width;
			windowHeight = (u32)videoMode->height;
			m_desc.size = { windowWidth, windowHeight };

			glfwWindowHint(GLFW_RED_BITS, videoMode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, videoMode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, videoMode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);
		}

		gWindow = glfwCreateWindow(windowWidth, windowHeight, m_desc.name.data(), nullptr, nullptr);
		if (!gWindow)
		{
			glfwTerminate();
			exit(-1);
		}

		if (borderless)
		{
			i32 monitorX, monitorY;
			glfwGetMonitorPos(monitor, &monitorX, &monitorY);
			glfwSetWindowPos(gWindow, monitorX, monitorY);
		}

		i32 framebufferWidth = 0, framebufferHeight = 0;
		glfwGetFramebufferSize(gWindow, &framebufferWidth, &framebufferHeight);
		m_desc.size = { (u32)framebufferWidth, (u32)framebufferHeight };

		glfwSetWindowUserPointer(gWindow, (void*)(&InputSystem()));

		m_osHandle = (void*)glfwGetWin32Window(gWindow);
		m_instance = nullptr;

		glfwSetCursorPosCallback(gWindow, MouseMoveCallback);
		glfwSetMouseButtonCallback(gWindow, MouseButtonCallback);
		glfwSetScrollCallback(gWindow, MouseWheelScrollCallback);
		glfwSetFramebufferSizeCallback(gWindow, ResizeCallback);
		glfwSetKeyCallback(gWindow, KeyboardCallback);
		glfwSetCharCallback(gWindow, CharCallback);
	}

	Window::~Window()
	{
		if (gWindow)
		{
			glfwSetWindowShouldClose(gWindow, GLFW_TRUE);
			glfwDestroyWindow(gWindow);
			glfwTerminate();
			gWindow = nullptr;
		}
	}

	void Window::Show()
	{
		if (gWindow)
			glfwShowWindow(gWindow);

		m_hidden = false;
	}

	void Window::Hide()
	{
		if (gWindow)
			glfwHideWindow(gWindow);

		m_hidden = true;
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	b8 Window::IsActive() const
	{
		return !glfwWindowShouldClose(gWindow);
	}

	void* Window::GetAPIHandle() const
	{
		return gWindow;
	}

}