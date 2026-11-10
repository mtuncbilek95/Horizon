#pragma once

#include <Runtime/Containers/Vector/List.h>

#include <Runtime/Math/Color4f.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	struct RenderableSpotLight
	{
		Math::Color4f color;
		Math::Vec3f position;
		Math::Vec3f direction;
		f32 intensity;
		f32 range;
		f32 innerConeAngle;
		f32 outerConeAngle;
	};

	struct FrameSpotLight
	{
		List<RenderableSpotLight> spotLights;
	};
}