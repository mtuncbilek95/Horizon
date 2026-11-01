#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class LoadOp : u8
	{
		Load,
		Clear,
		DontCare
	};
}
