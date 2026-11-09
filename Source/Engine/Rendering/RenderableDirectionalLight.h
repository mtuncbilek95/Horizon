#pragma once

#include <Runtime/Math/Color4f.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	struct RenderableDirectionalLight
	{
		Math::Color4f color;
		Math::Vec3f direction;
		f32 intensity;
	};
}