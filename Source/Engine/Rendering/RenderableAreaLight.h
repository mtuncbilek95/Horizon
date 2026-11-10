#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Math/Color4f.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	struct RenderableAreaLight
	{
		Math::Color4f color;
		Math::Vec3f position;
		Math::Vec3f right;
		Math::Vec3f up;
		f32 intensity;
		f32 width;
		f32 height;
		b8 twoSided;
	};

	struct FrameAreaLight
	{
		List<RenderableAreaLight> areaLights;
	};
}