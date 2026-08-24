#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	struct H_EXPORT StaticMeshVertex
	{
		f32 position[3] = {};
		f32 normal[3] = {};
		f32 tangent[4] = {};
		f32 uv[2] = {};
	};
}