#include "ImGuiUtils.h"

namespace Horizon::Editor
{
	ImGuiKey ImGuiUtils::GetKeyboardKey(const PAL::KeyCode key)
	{
		switch (key)
		{
		case PAL::KeyCode::Unknown:
			break;
		case PAL::KeyCode::Space:
			return ImGuiKey_Space;
		case PAL::KeyCode::Apostrophe:
			return ImGuiKey_Apostrophe;
		case PAL::KeyCode::Comma:
			return ImGuiKey_Comma;
		case PAL::KeyCode::Minus:
			return ImGuiKey_Minus;
		case PAL::KeyCode::Period:
			return ImGuiKey_Period;
		case PAL::KeyCode::Slash:
			return ImGuiKey_Slash;
		case PAL::KeyCode::Num0:
			return ImGuiKey_0;
		case PAL::KeyCode::Num1:
			return ImGuiKey_1;
		case PAL::KeyCode::Num2:
			return ImGuiKey_2;
		case PAL::KeyCode::Num3:
			return ImGuiKey_3;
		case PAL::KeyCode::Num4:
			return ImGuiKey_4;
		case PAL::KeyCode::Num5:
			return ImGuiKey_5;
		case PAL::KeyCode::Num6:
			return ImGuiKey_6;
		case PAL::KeyCode::Num7:
			return ImGuiKey_7;
		case PAL::KeyCode::Num8:
			return ImGuiKey_8;
		case PAL::KeyCode::Num9:
			return ImGuiKey_9;
		case PAL::KeyCode::Semicolon:
			return ImGuiKey_Semicolon;
		case PAL::KeyCode::Equal:
			return ImGuiKey_Equal;
		case PAL::KeyCode::A:
			return ImGuiKey_A;
		case PAL::KeyCode::B:
			return ImGuiKey_B;
		case PAL::KeyCode::C:
			return ImGuiKey_C;
		case PAL::KeyCode::D:
			return ImGuiKey_D;
		case PAL::KeyCode::E:
			return ImGuiKey_E;
		case PAL::KeyCode::F:
			return ImGuiKey_F;
		case PAL::KeyCode::G:
			return ImGuiKey_G;
		case PAL::KeyCode::H:
			return ImGuiKey_H;
		case PAL::KeyCode::I:
			return ImGuiKey_I;
		case PAL::KeyCode::J:
			return ImGuiKey_J;
		case PAL::KeyCode::K:
			return ImGuiKey_K;
		case PAL::KeyCode::L:
			return ImGuiKey_L;
		case PAL::KeyCode::M:
			return ImGuiKey_M;
		case PAL::KeyCode::N:
			return ImGuiKey_N;
		case PAL::KeyCode::O:
			return ImGuiKey_O;
		case PAL::KeyCode::P:
			return ImGuiKey_P;
		case PAL::KeyCode::Q:
			return ImGuiKey_Q;
		case PAL::KeyCode::R:
			return ImGuiKey_R;
		case PAL::KeyCode::S:
			return ImGuiKey_S;
		case PAL::KeyCode::T:
			return ImGuiKey_T;
		case PAL::KeyCode::U:
			return ImGuiKey_U;
		case PAL::KeyCode::V:
			return ImGuiKey_V;
		case PAL::KeyCode::W:
			return ImGuiKey_W;
		case PAL::KeyCode::X:
			return ImGuiKey_X;
		case PAL::KeyCode::Y:
			return ImGuiKey_Y;
		case PAL::KeyCode::Z:
			return ImGuiKey_Z;
		case PAL::KeyCode::LeftBracket:
			return ImGuiKey_LeftBracket;
		case PAL::KeyCode::Backslash:
			return ImGuiKey_Backslash;
		case PAL::KeyCode::RightBracket:
			return ImGuiKey_RightBracket;
		case PAL::KeyCode::GraveAccent:
			return ImGuiKey_GraveAccent;
		case PAL::KeyCode::Escape:
			return ImGuiKey_Escape;
		case PAL::KeyCode::Enter:
			return ImGuiKey_Enter;
		case PAL::KeyCode::Tab:
			return ImGuiKey_Tab;
		case PAL::KeyCode::Backspace:
			return ImGuiKey_Backspace;
		case PAL::KeyCode::Insert:
			return ImGuiKey_Insert;
		case PAL::KeyCode::Delete:
			return ImGuiKey_Delete;
		case PAL::KeyCode::Right:
			return ImGuiKey_RightArrow;
		case PAL::KeyCode::Left:
			return ImGuiKey_LeftArrow;
		case PAL::KeyCode::Down:
			return ImGuiKey_DownArrow;
		case PAL::KeyCode::Up:
			return ImGuiKey_UpArrow;
		case PAL::KeyCode::PageUp:
			return ImGuiKey_PageUp;
		case PAL::KeyCode::PageDown:
			return ImGuiKey_PageDown;
		case PAL::KeyCode::Home:
			return ImGuiKey_Home;
		case PAL::KeyCode::End:
			return ImGuiKey_End;
		case PAL::KeyCode::CapsLock:
			return ImGuiKey_CapsLock;
		case PAL::KeyCode::ScrollLock:
			return ImGuiKey_ScrollLock;
		case PAL::KeyCode::NumLock:
			return ImGuiKey_NumLock;
		case PAL::KeyCode::PrintScreen:
			return ImGuiKey_PrintScreen;
		case PAL::KeyCode::Pause:
			return ImGuiKey_Pause;
		case PAL::KeyCode::F1:
			return ImGuiKey_F1;
		case PAL::KeyCode::F2:
			return ImGuiKey_F2;
		case PAL::KeyCode::F3:
			return ImGuiKey_F3;
		case PAL::KeyCode::F4:
			return ImGuiKey_F4;
		case PAL::KeyCode::F5:
			return ImGuiKey_F5;
		case PAL::KeyCode::F6:
			return ImGuiKey_F6;
		case PAL::KeyCode::F7:
			return ImGuiKey_F7;
		case PAL::KeyCode::F8:
			return ImGuiKey_F8;
		case PAL::KeyCode::F9:
			return ImGuiKey_F9;
		case PAL::KeyCode::F10:
			return ImGuiKey_F10;
		case PAL::KeyCode::F11:
			return ImGuiKey_F11;
		case PAL::KeyCode::F12:
			return ImGuiKey_F12;
		case PAL::KeyCode::Kp0:
			return ImGuiKey_Keypad0;
		case PAL::KeyCode::Kp1:
			return ImGuiKey_Keypad1;
		case PAL::KeyCode::Kp2:
			return ImGuiKey_Keypad2;
		case PAL::KeyCode::Kp3:
			return ImGuiKey_Keypad3;
		case PAL::KeyCode::Kp4:
			return ImGuiKey_Keypad4;
		case PAL::KeyCode::Kp5:
			return ImGuiKey_Keypad5;
		case PAL::KeyCode::Kp6:
			return ImGuiKey_Keypad6;
		case PAL::KeyCode::Kp7:
			return ImGuiKey_Keypad7;
		case PAL::KeyCode::Kp8:
			return ImGuiKey_Keypad8;
		case PAL::KeyCode::Kp9:
			return ImGuiKey_Keypad9;
		case PAL::KeyCode::KpDecimal:
			return ImGuiKey_KeypadDecimal;
		case PAL::KeyCode::KpDivide:
			return ImGuiKey_KeypadDivide;
		case PAL::KeyCode::KpMultiply:
			return ImGuiKey_KeypadMultiply;
		case PAL::KeyCode::KpSubtract:
			return ImGuiKey_KeypadSubtract;
		case PAL::KeyCode::KpAdd:
			return ImGuiKey_KeypadAdd;
		case PAL::KeyCode::KpEnter:
			return ImGuiKey_KeypadEnter;
		case PAL::KeyCode::KpEqual:
			return ImGuiKey_KeypadEqual;
		case PAL::KeyCode::LeftShift:
			return ImGuiKey_LeftShift;
		case PAL::KeyCode::LeftControl:
			return ImGuiKey_LeftCtrl;
		case PAL::KeyCode::LeftAlt:
			return ImGuiKey_LeftAlt;
		case PAL::KeyCode::LeftSuper:
			return ImGuiKey_LeftSuper;
		case PAL::KeyCode::RightShift:
			return ImGuiKey_RightShift;
		case PAL::KeyCode::RightControl:
			return ImGuiKey_RightCtrl;
		case PAL::KeyCode::RightAlt:
			return ImGuiKey_RightAlt;
		case PAL::KeyCode::RightSuper:
			return ImGuiKey_RightSuper;
		case PAL::KeyCode::Menu:
			return ImGuiKey_Menu;
		default:
			return ImGuiKey_None;
		}

		return ImGuiKey_None;
	}

	ImGuiKey ImGuiUtils::GetModifierKey(const PAL::KeyCode key)
	{
		switch (key)
		{
		case PAL::KeyCode::LeftControl:
		case PAL::KeyCode::RightControl:
			return ImGuiMod_Ctrl;
		case PAL::KeyCode::LeftShift:
		case PAL::KeyCode::RightShift:
			return ImGuiMod_Shift;
		case PAL::KeyCode::LeftAlt:
		case PAL::KeyCode::RightAlt:
			return ImGuiMod_Alt;
		case PAL::KeyCode::LeftSuper:
		case PAL::KeyCode::RightSuper:
			return ImGuiMod_Super;
		default:
			return ImGuiKey_None;
		}
	}

	ImGuiMouseButton ImGuiUtils::GetMouseButton(const PAL::MouseButton button)
	{
		switch (button)
		{
		case PAL::MouseButton::Right:
		default:
			return ImGuiMouseButton_Right;
		case PAL::MouseButton::Left:
			return ImGuiMouseButton_Left;
		case PAL::MouseButton::Middle:
			return ImGuiMouseButton_Middle;
		}
	}

	ImVec4 ImGuiUtils::Hex(std::string_view hex)
	{
		auto nib = [](char c) -> u32
			{
				if (c >= '0' && c <= '9') return u32(c - '0');
				if (c >= 'a' && c <= 'f') return u32(c - 'a' + 10);
				if (c >= 'A' && c <= 'F') return u32(c - 'A' + 10);
				return 0u;
			};

		usize i = (!hex.empty() && hex[0] == '#') ? 1u : 0u;

		auto byteAt = [&](usize idx) -> f32
			{
				return f32((nib(hex[idx]) << 4) | nib(hex[idx + 1])) / 255.0f;
			};

		f32 r = byteAt(i + 0);
		f32 g = byteAt(i + 2);
		f32 b = byteAt(i + 4);
		f32 a = (hex.size() - i >= 8) ? byteAt(i + 6) : 1.0f;

		return ImVec4(r, g, b, a);
	}
}