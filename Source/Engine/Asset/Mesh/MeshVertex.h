#pragma once

#include <Runtime/Math/Vec4f.h>
#include <Runtime/Math/Vec2f.h>

namespace Horizon::Engine
{
	struct MeshVertex
	{
		Math::Vec4f position;
		Math::Vec4f normal;
		Math::Vec4f tangent;
		Math::Vec4f color;
		Math::Vec2f uv;
	};
}