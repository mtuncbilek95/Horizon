#pragma once

#include <Runtime/PAL/Window/InputMessageType.h>
#include <Runtime/PAL/Window/InputModifiers.h>
#include <Runtime/PAL/Window/KeyCode.h>
#include <Runtime/PAL/Window/MouseButton.h>
#include <Runtime/PAL/Window/WindowMode.h>

#include <vector>
#include <filesystem>

namespace Horizon::PAL
{
	struct InputMessage
	{
		InputMessageType type;
		
		KeyCode key;
		u32 character;
		
		MouseButton button;
		i32 mouseX, mouseY;
		f32 scrollX, scrollY;

		u32 width, height;
		i32 posX, posY;

		InputModifiers modifiers;

		std::vector<std::filesystem::path> filePaths;
	};
}
