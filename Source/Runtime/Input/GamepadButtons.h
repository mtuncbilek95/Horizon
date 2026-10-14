#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Input
{
	HENUM();
	enum class GamepadButtons : u32
	{
		None = 0x0000,
		Menu = 0x0001,
		View = 0x0002,
		South = 0x0004,
		East = 0x0008,
		West = 0x0010,
		North = 0x0020,
		DPadUp = 0x0040,
		DPadDown = 0x0080,
		DPadLeft = 0x0100,
		DPadRight = 0x0200,
		LeftShoulder = 0x0400,
		RightShoulder = 0x0800,
		LeftThumb = 0x1000,
		RightThumb = 0x2000
	};
}