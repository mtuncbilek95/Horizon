#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class GfxPrimitiveTopology : u8
	{
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip
	};
}
