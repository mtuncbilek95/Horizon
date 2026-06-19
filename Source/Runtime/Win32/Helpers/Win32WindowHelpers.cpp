#include <Runtime/Win32/Helpers/Win32WindowHelpers.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>

namespace Horizon
{
	KeyCode WindowHelpers::ToWinKey(u64 virtualKey)
	{
		if ((virtualKey >= 'A' && virtualKey <= 'Z') || (virtualKey >= '0' && virtualKey <= '9'))
			return KeyCode(virtualKey);

		switch (virtualKey)
		{
			case VK_SPACE: return KeyCode::Space;
			case VK_OEM_7: return KeyCode::Apostrophe;
			case VK_OEM_COMMA: return KeyCode::Comma;
			case VK_OEM_MINUS: return KeyCode::Minus;
			case VK_OEM_PERIOD: return KeyCode::Period;
			case VK_OEM_2: return KeyCode::Slash;
			case VK_OEM_1: return KeyCode::Semicolon;
			case VK_OEM_PLUS: return KeyCode::Equal;
			case VK_OEM_4: return KeyCode::LeftBracket;
			case VK_OEM_5: return KeyCode::Backslash;
			case VK_OEM_6: return KeyCode::RightBracket;
			case VK_OEM_3: return KeyCode::GraveAccent;
			case VK_ESCAPE: return KeyCode::Escape;
			case VK_RETURN: return KeyCode::Enter;
			case VK_TAB: return KeyCode::Tab;
			case VK_BACK: return KeyCode::Backspace;
			case VK_INSERT: return KeyCode::Insert;
			case VK_DELETE: return KeyCode::Delete;
			case VK_RIGHT: return KeyCode::Right;
			case VK_LEFT: return KeyCode::Left;
			case VK_DOWN: return KeyCode::Down;
			case VK_UP: return KeyCode::Up;
			case VK_PRIOR: return KeyCode::PageUp;
			case VK_NEXT: return KeyCode::PageDown;
			case VK_HOME: return KeyCode::Home;
			case VK_END: return KeyCode::End;
			case VK_CAPITAL: return KeyCode::CapsLock;
			case VK_SCROLL: return KeyCode::ScrollLock;
			case VK_NUMLOCK: return KeyCode::NumLock;
			case VK_SNAPSHOT: return KeyCode::PrintScreen;
			case VK_PAUSE: return KeyCode::Pause;
			case VK_F1: return KeyCode::F1;
			case VK_F2: return KeyCode::F2;
			case VK_F3: return KeyCode::F3;
			case VK_F4: return KeyCode::F4;
			case VK_F5: return KeyCode::F5;
			case VK_F6: return KeyCode::F6;
			case VK_F7: return KeyCode::F7;
			case VK_F8: return KeyCode::F8;
			case VK_F9: return KeyCode::F9;
			case VK_F10: return KeyCode::F10;
			case VK_F11: return KeyCode::F11;
			case VK_F12: return KeyCode::F12;
			case VK_NUMPAD0: return KeyCode::Kp0;
			case VK_NUMPAD1: return KeyCode::Kp1;
			case VK_NUMPAD2: return KeyCode::Kp2;
			case VK_NUMPAD3: return KeyCode::Kp3;
			case VK_NUMPAD4: return KeyCode::Kp4;
			case VK_NUMPAD5: return KeyCode::Kp5;
			case VK_NUMPAD6: return KeyCode::Kp6;
			case VK_NUMPAD7: return KeyCode::Kp7;
			case VK_NUMPAD8: return KeyCode::Kp8;
			case VK_NUMPAD9: return KeyCode::Kp9;
			case VK_DECIMAL: return KeyCode::KpDecimal;
			case VK_DIVIDE: return KeyCode::KpDivide;
			case VK_MULTIPLY: return KeyCode::KpMultiply;
			case VK_SUBTRACT: return KeyCode::KpSubtract;
			case VK_ADD: return KeyCode::KpAdd;
			case VK_LSHIFT: return KeyCode::LeftShift;
			case VK_RSHIFT: return KeyCode::RightShift;
			case VK_LCONTROL: return KeyCode::LeftControl;
			case VK_RCONTROL: return KeyCode::RightControl;
			case VK_LMENU: return KeyCode::LeftAlt;
			case VK_RMENU: return KeyCode::RightAlt;
			case VK_LWIN: return KeyCode::LeftSuper;
			case VK_RWIN: return KeyCode::RightSuper;
			case VK_APPS: return KeyCode::Menu;
			case VK_SHIFT: return KeyCode::LeftShift;
			case VK_CONTROL: return KeyCode::LeftControl;
			case VK_MENU: return KeyCode::LeftAlt;
			default: return KeyCode::Unknown;
		}
	}

	MouseButton WindowHelpers::ToWinMouse(u32 message, u64 wParam)
	{
		switch (message)
		{
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
				return MouseButton::Left;
			case WM_RBUTTONDOWN:
			case WM_RBUTTONUP:
				return MouseButton::Right;
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
				return MouseButton::Middle;
			case WM_XBUTTONDOWN:
			case WM_XBUTTONUP:
				return GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? MouseButton::Button4 : MouseButton::Button5;
		}

		return MouseButton::Left;
	}

	InputModifiers WindowHelpers::ToWinModifiers()
	{
		InputModifiers modifiers = InputModifiers::None;

		if (GetKeyState(VK_SHIFT) & 0x8000)
			modifiers |= InputModifiers::Shift;
		if (GetKeyState(VK_CONTROL) & 0x8000)
			modifiers |= InputModifiers::Control;
		if (GetKeyState(VK_MENU) & 0x8000)
			modifiers |= InputModifiers::Alt;
		if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
			modifiers |= InputModifiers::Super;
		if (GetKeyState(VK_CAPITAL) & 0x0001)
			modifiers |= InputModifiers::CapsLock;
		if (GetKeyState(VK_NUMLOCK) & 0x0001)
			modifiers |= InputModifiers::NumLock;

		return modifiers;
	}

	u64 WindowHelpers::FromWinKey(KeyCode key)
	{
		if ((key >= KeyCode::A && key <= KeyCode::Z) || (key >= KeyCode::Num0 && key <= KeyCode::Num9))
			return u64(key);

		switch (key)
		{
			case KeyCode::Space: return VK_SPACE;
			case KeyCode::Apostrophe: return VK_OEM_7;
			case KeyCode::Comma: return VK_OEM_COMMA;
			case KeyCode::Minus: return VK_OEM_MINUS;
			case KeyCode::Period: return VK_OEM_PERIOD;
			case KeyCode::Slash: return VK_OEM_2;
			case KeyCode::Semicolon: return VK_OEM_1;
			case KeyCode::Equal: return VK_OEM_PLUS;
			case KeyCode::LeftBracket: return VK_OEM_4;
			case KeyCode::Backslash: return VK_OEM_5;
			case KeyCode::RightBracket: return VK_OEM_6;
			case KeyCode::GraveAccent: return VK_OEM_3;
			case KeyCode::Escape: return VK_ESCAPE;
			case KeyCode::Enter: return VK_RETURN;
			case KeyCode::Tab: return VK_TAB;
			case KeyCode::Backspace: return VK_BACK;
			case KeyCode::Insert: return VK_INSERT;
			case KeyCode::Delete: return VK_DELETE;
			case KeyCode::Right: return VK_RIGHT;
			case KeyCode::Left: return VK_LEFT;
			case KeyCode::Down: return VK_DOWN;
			case KeyCode::Up: return VK_UP;
			case KeyCode::PageUp: return VK_PRIOR;
			case KeyCode::PageDown: return VK_NEXT;
			case KeyCode::Home: return VK_HOME;
			case KeyCode::End: return VK_END;
			case KeyCode::CapsLock: return VK_CAPITAL;
			case KeyCode::ScrollLock: return VK_SCROLL;
			case KeyCode::NumLock: return VK_NUMLOCK;
			case KeyCode::PrintScreen: return VK_SNAPSHOT;
			case KeyCode::Pause: return VK_PAUSE;
			case KeyCode::F1: return VK_F1;
			case KeyCode::F2: return VK_F2;
			case KeyCode::F3: return VK_F3;
			case KeyCode::F4: return VK_F4;
			case KeyCode::F5: return VK_F5;
			case KeyCode::F6: return VK_F6;
			case KeyCode::F7: return VK_F7;
			case KeyCode::F8: return VK_F8;
			case KeyCode::F9: return VK_F9;
			case KeyCode::F10: return VK_F10;
			case KeyCode::F11: return VK_F11;
			case KeyCode::F12: return VK_F12;
			case KeyCode::Kp0: return VK_NUMPAD0;
			case KeyCode::Kp1: return VK_NUMPAD1;
			case KeyCode::Kp2: return VK_NUMPAD2;
			case KeyCode::Kp3: return VK_NUMPAD3;
			case KeyCode::Kp4: return VK_NUMPAD4;
			case KeyCode::Kp5: return VK_NUMPAD5;
			case KeyCode::Kp6: return VK_NUMPAD6;
			case KeyCode::Kp7: return VK_NUMPAD7;
			case KeyCode::Kp8: return VK_NUMPAD8;
			case KeyCode::Kp9: return VK_NUMPAD9;
			case KeyCode::KpDecimal: return VK_DECIMAL;
			case KeyCode::KpDivide: return VK_DIVIDE;
			case KeyCode::KpMultiply: return VK_MULTIPLY;
			case KeyCode::KpSubtract: return VK_SUBTRACT;
			case KeyCode::KpAdd: return VK_ADD;
			case KeyCode::LeftShift: return VK_LSHIFT;
			case KeyCode::RightShift: return VK_RSHIFT;
			case KeyCode::LeftControl: return VK_LCONTROL;
			case KeyCode::RightControl: return VK_RCONTROL;
			case KeyCode::LeftAlt: return VK_LMENU;
			case KeyCode::RightAlt: return VK_RMENU;
			case KeyCode::LeftSuper: return VK_LWIN;
			case KeyCode::RightSuper: return VK_RWIN;
			case KeyCode::Menu: return VK_APPS;
			default: return 0;
		}
	}

	u64 WindowHelpers::FromWinMouse(MouseButton button)
	{
		switch (button)
		{
			case MouseButton::Left: return VK_LBUTTON;
			case MouseButton::Right: return VK_RBUTTON;
			case MouseButton::Middle: return VK_MBUTTON;
			case MouseButton::Button4: return VK_XBUTTON1;
			case MouseButton::Button5: return VK_XBUTTON2;
			default: return 0;
		}
	}

	u32 WindowHelpers::FromWinModifiers(InputModifiers modifiers)
	{
		u32 mask = 0;

		if (HasFlag(modifiers, InputModifiers::Shift))
			mask |= MOD_SHIFT;
		if (HasFlag(modifiers, InputModifiers::Control))
			mask |= MOD_CONTROL;
		if (HasFlag(modifiers, InputModifiers::Alt))
			mask |= MOD_ALT;
		if (HasFlag(modifiers, InputModifiers::Super))
			mask |= MOD_WIN;

		return mask;
	}
}
