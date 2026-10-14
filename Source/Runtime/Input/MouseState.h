#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Input
{
	struct MouseState
	{
		u32 buttons = 0;
		i64 positionX = 0;
		i64 positionY = 0;
		i64 wheelX = 0;
		i64 wheelY = 0;
	};
}