#pragma once

#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	struct RenderFrameData
	{
		Math::Mat4f view;
		Math::Mat4f proj;
		Math::Mat4f viewProj;
		Math::Vec3f cameraPosition;
		f32 time = 0.f;
	};
}