#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	struct GfxViewport
	{
		f32 x = 0.0f;
		f32 y = 0.0f;
		f32 width = 0.0f;
		f32 height = 0.0f;
		f32 minDepth = 0.0f;
		f32 maxDepth = 1.0f;
	};
}