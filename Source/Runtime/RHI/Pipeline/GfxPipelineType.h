#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class GfxPipelineType : u8
	{
		Graphics,
		Compute,
		Raytracing
	};
}