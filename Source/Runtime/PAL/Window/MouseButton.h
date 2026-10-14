#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::PAL
{
	HENUM();
	enum class MouseButton : u64
	{
		Left,
		Right,
		Middle,
		Button4,
		Button5,
		Button6,
		Button7,
		Button8
	};
}
