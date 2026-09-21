#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	struct MeshSubMesh
	{
		u32 indexOffset;
		u32 indexCount;
		u32 vertexOffset;
		u32 vertexCount;
		u32 materialIndex;
		Math::Vec3f boundsMin;
		Math::Vec3f boundsMax;
	};
}
