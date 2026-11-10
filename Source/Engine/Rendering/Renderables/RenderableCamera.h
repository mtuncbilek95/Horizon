#pragma once

#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	struct RenderableCamera
	{
		Math::Mat4f camViewMatrix;
		Math::Mat4f camProjMatrix;
		Math::Mat4f camViewProjMatrix;
		Math::Vec3f camWorldPosition;
		f32 nearPlane;
		f32 farPlane;
	};
}