#pragma once

#include <Runtime/PAL/Window/KeyCode.h>
#include <Runtime/PAL/Window/MouseButton.h>
#include <Runtime/PAL/Window/InputModifiers.h>

namespace Horizon::PAL
{
	struct WindowHelpers
	{
		static KeyCode ToWinKey(u64 virtualKey);
		static MouseButton ToWinMouse(u32 message, u64 wParam);
		static InputModifiers ToWinModifiers();

		static u64 FromWinKey(KeyCode key);
		static u64 FromWinMouse(MouseButton button);
		static u32 FromWinModifiers(InputModifiers modifiers);
	};
}
