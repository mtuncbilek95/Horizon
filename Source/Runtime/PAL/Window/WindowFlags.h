#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::PAL
{
	enum class WindowFlags : u32
	{
		None = 0,
		EnableDragDrop = 1 << 0,
		CustomTitleBar = 1 << 1,
		NoDecoration = 1 << 2,
		NoTaskbarIcon = 1 << 3,
		TopMost = 1 << 4,
		NoActivate = 1 << 5,
		PassThroughInput = 1 << 6
	};
}