#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class GfxLoadOp : u8
	{
		Load,
		Clear,
		DontCare
	};
}
