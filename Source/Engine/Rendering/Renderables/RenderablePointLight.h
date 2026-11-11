#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Math/Color4f.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	static constexpr u32 kMaxPointLightCount = 8196;

	struct RenderablePointLight
	{
		Math::Color4f color;
		Math::Vec3f position;
		f32 intensity;
		f32 range;
	};

	struct FramePointLight
	{
		List<RenderablePointLight> pointLights;
	};
}