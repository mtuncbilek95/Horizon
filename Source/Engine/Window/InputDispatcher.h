#pragma once

#include <Engine/Window/InputEvents.h>

#include <functional>
#include <vector>
#include <bitset>

namespace Horizon
{
	class InputDispatcher
	{
	public:
		using InputCallbackFn = std::function<void(const InputMessage&)>;

		void OnKeyPressed(InputCallbackFn callback);
		void OnMousePressed(InputCallbackFn callback);
		void OnResizeWindow(InputCallbackFn callback);

		void DispatchKey(KeyCode key, i32 scancode, InputAction action, InputModifiers modifiers);
		void DispatchMouseButton(MouseButton button, InputAction action, InputModifiers modifiers);
		void DispatchMouseMove(double positionX, double positionY);
		void DispatchMouseScroll(double offsetX, double offsetY);
		void DispatchChar(u32 codepoint);
		void DispatchResize(i32 width, i32 height);

		bool IsKeyDown(KeyCode key) const;
		bool IsMouseButtonDown(MouseButton button) const;
		double MouseX() const { return m_mouseX; }
		double MouseY() const { return m_mouseY; }
		InputModifiers Modifiers() const { return m_modifiers; }
		i32 ButtonsDown() const { return m_buttonsDown; }

	private:
		InputMessage BuildBase(InputType type) const;

		InputModifiers m_modifiers = InputModifiers::None;
		double m_mouseX = 0.0;
		double m_mouseY = 0.0;
		i32 m_buttonsDown = 0;
		std::bitset<512> m_keysDown;

		std::vector<InputCallbackFn> m_keyCallbacks;
		std::vector<InputCallbackFn> m_mouseCallbacks;
		std::vector<InputCallbackFn> m_windowCallbacks;
	};

	inline InputDispatcher& InputSystem()
	{
		static InputDispatcher instance;
		return instance;
	}
}
