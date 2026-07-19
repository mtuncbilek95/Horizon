#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::PAL
{
	enum class WindowFlags : u32
	{
		None = 0,
		EnableDragDrop = 1 << 0
	};
}