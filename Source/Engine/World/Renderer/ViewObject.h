#pragma once

namespace Horizon
{
	struct ViewObject
	{
		Math::Mat4f viewMatrix;
		Math::Mat4f projMatrix;
		Math::Vec3f cameraPosition;
	};
}