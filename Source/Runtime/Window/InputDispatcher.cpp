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
		msg.buttonsDown = m_buttonsDown;
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

	void InputDispatcher::OnResizeWindow(InputCallbackFn callback)
	{
		m_windowCallbacks.push_back(std::move(callback));
	}

	bool InputDispatcher::IsKeyDown(KeyCode key) const
	{
		i32 keyIndex = static_cast<i32>(key);
		if (keyIndex < 0 || static_cast<size_t>(keyIndex) >= m_keysDown.size())
			return false;
		return m_keysDown.test(static_cast<size_t>(keyIndex));
	}

	bool InputDispatcher::IsMouseButtonDown(MouseButton button) const
	{
		return (m_buttonsDown & (1 << static_cast<i32>(button))) != 0;
	}

	void InputDispatcher::DispatchKey(KeyCode key, i32 scancode, InputAction action, InputModifiers modifiers)
	{
		m_modifiers = modifiers;

		i32 keyIndex = static_cast<i32>(key);
		if (keyIndex >= 0 && static_cast<size_t>(keyIndex) < m_keysDown.size())
		{
			if (action == InputAction::Press)
				m_keysDown.set(static_cast<size_t>(keyIndex));
			else if (action == InputAction::Release)
				m_keysDown.reset(static_cast<size_t>(keyIndex));
		}

		InputMessage msg = BuildBase(InputType::Key);
		msg.key = key;
		msg.scancode = scancode;
		msg.keyAction = action;

		for (const auto& callback : m_keyCallbacks)
			callback(msg);
	}

	void InputDispatcher::DispatchMouseButton(MouseButton button, InputAction action, InputModifiers modifiers)
	{
		m_modifiers = modifiers;

		if (action == InputAction::Press)
			m_buttonsDown |= (1 << static_cast<i32>(button));
		else if (action == InputAction::Release)
			m_buttonsDown &= ~(1 << static_cast<i32>(button));

		InputMessage msg = BuildBase(InputType::MouseButton);
		msg.button = button;
		msg.mouseAction = action;

		for (const auto& callback : m_mouseCallbacks)
			callback(msg);
	}

	void InputDispatcher::DispatchMouseMove(double positionX, double positionY)
	{
		InputMessage msg = BuildBase(InputType::MouseMove);
		msg.mouseDX = positionX - m_mouseX;
		msg.mouseDY = positionY - m_mouseY;
		msg.mouseX = positionX;
		msg.mouseY = positionY;

		m_mouseX = positionX;
		m_mouseY = positionY;

		for (const auto& callback : m_mouseCallbacks)
			callback(msg);
	}

	void InputDispatcher::DispatchMouseScroll(double offsetX, double offsetY)
	{
		InputMessage msg = BuildBase(InputType::MouseScroll);
		msg.scrollX = offsetX;
		msg.scrollY = offsetY;

		for (const auto& callback : m_mouseCallbacks)
			callback(msg);
	}

	void InputDispatcher::DispatchChar(u32 codepoint)
	{
		InputMessage msg = BuildBase(InputType::Char);
		msg.codepoint = codepoint;

		for (const auto& callback : m_keyCallbacks)
			callback(msg);
	}

	void InputDispatcher::DispatchResize(i32 width, i32 height)
	{
		InputMessage msg = BuildBase(InputType::Resize);
		msg.resizeWidth = width;
		msg.resizeHeight = height;

		for (const auto& callback : m_windowCallbacks)
			callback(msg);
	}

}