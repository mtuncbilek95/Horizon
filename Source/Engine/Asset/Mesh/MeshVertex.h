#pragma once

#include <Runtime/Math/Vec4f.h>
#include <Runtime/Math/Vec2f.h>

namespace Horizon::Engine
{
	struct MeshVertex
	{
		Math::Vec3f position;
		Math::Vec3f normal;
		Math::Vec4f tangent;
		Math::Vec4f color;
		Math::Vec2f uv;
	};
}