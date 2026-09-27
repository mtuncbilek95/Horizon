#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	struct MeshDrawRange
	{
		u32 firstVertex = 0;
		u32 firstIndex = 0;
		u32 indexCount = 0;
	};
}