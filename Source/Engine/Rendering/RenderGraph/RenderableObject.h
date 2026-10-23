#pragma once

#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Vec2u.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine 
{
	struct RenderableObject
	{
		u32 firstVertex;
		u32 firstIndex;
		u32 indexCount;
		Math::Mat4f worldMatrix;
	};
}