#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::RHI
{
	HENUM();
	enum class SampleCount : u8
	{
		Count1 = 1,
		Count2 = 2,
		Count4 = 4,
		Count8 = 8
	};
}