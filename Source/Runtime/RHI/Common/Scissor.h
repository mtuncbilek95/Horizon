#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	struct Scissor final
	{
		i32 x = 0;
		i32 y = 0;
		i32 width = 0;
		i32 height = 0;
	};
}
