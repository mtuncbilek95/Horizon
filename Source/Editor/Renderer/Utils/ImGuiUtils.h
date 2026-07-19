#pragma once

#include <Runtime/PAL/Window/KeyCode.h>
#include <Runtime/PAL/Window/MouseButton.h>

#include <imgui.h>
#include <string_view>

namespace Horizon
{
	struct H_EXPORT ImGuiUtils final
	{
		static ImGuiKey GetKeyboardKey(const PAL::KeyCode key);
		static ImGuiKey GetModifierKey(const PAL::KeyCode key);
		static ImGuiMouseButton GetMouseButton(const PAL::MouseButton button);
		static ImVec4 Hex(std::string_view hex);
	};
}