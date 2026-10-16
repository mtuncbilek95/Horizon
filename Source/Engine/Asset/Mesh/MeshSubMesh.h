#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Math/BoundingBox.h>

namespace Horizon::Engine
{
	struct MeshSubMesh
	{
		u32 indexOffset;
		u32 indexCount;
		u32 vertexOffset;
		u32 vertexCount;
		Math::BoundingBox boundingBox;
	};
}
