#pragma once

#include <Runtime/Math/Color4f.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	struct RenderableDirectionalLight
	{
		Math::Color4f color = Math::Color4f::White();
		Math::Vec3f direction = { 0.f, 0.f, -1.f };
		f32 illuminance = 0.0f;
	};
}