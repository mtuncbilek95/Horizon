#pragma once

#include <Runtime/PAL/Window/KeyCode.h>
#include <Runtime/PAL/Window/MouseButton.h>

#include <imgui.h>

namespace Horizon
{
	struct H_EXPORT ImGuiUtils final
	{
		static ImGuiKey GetKeyboardKey(const PAL::KeyCode key);
		static ImGuiKey GetModifierKey(const PAL::KeyCode key);
		static ImGuiMouseButton GetMouseButton(const PAL::MouseButton button);
	};
}