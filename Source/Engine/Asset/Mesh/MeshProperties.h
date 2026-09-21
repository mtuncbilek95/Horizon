#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	struct MeshProperties
	{
		u32 vertexCount;
		u32 indexCount;
		u32 vertexStride;
		u32 indexStride;
		u32 subMeshCount;
	};
}
