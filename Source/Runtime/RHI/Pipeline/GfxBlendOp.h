#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class GfxBlendOp : u8
	{
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};
}
