#pragma once

#include <Editor/Views/ViewCommand.h>

#include <imgui.h>

namespace Horizon::Editor
{
	struct ViewCommandBinding
	{
		ViewCommand command;
		ImGuiKeyChord focusedChord;
		ImGuiKeyChord broadcastChord;
	};

	inline constexpr ViewCommandBinding ViewCommandBindings[] =
	{
		{ ViewCommand::Save, ImGuiMod_Ctrl | ImGuiKey_S, ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S },
	};
}