#pragma once

#include <Engine/Window/InputAction.h>
#include <Engine/Window/InputKeyCode.h>
#include <Engine/Window/InputModifiers.h>
#include <Engine/Window/InputMouseButton.h>

namespace Horizon
{
	enum class InputType : u32
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
		f64 mouseDX = 0.0;
		f64 mouseDY = 0.0;
		i32 buttonsDown = 0;

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
