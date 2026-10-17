#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Math/BoundingBox.h>

namespace Horizon::Engine
{
	struct MeshAssetSubmesh
	{
		u32 indexOffset = 0;
		u32 indexCount = 0;
		u32 vertexOffset = 0;
		u32 vertexCount = 0;
		u32 materialSlotIndex = 0;
		Math::BoundingBox boundingBox = Math::BoundingBox::Empty();
	};

	struct MeshAssetProperties
	{
		u32 vertexCount = 0;
		u32 indexCount = 0;
		u32 vertexStride = 0;
		u32 indexStride = 0;
		u32 subMeshCount = 0;
		Math::BoundingBox mainBounds;
	};
}
