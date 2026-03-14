#pragma once

#include <Runtime/Data/Containers/Math.h>

namespace Horizon
{
	struct TransformComponent
	{
		Math::Vec3f position = { 0.f, 0.f, 0.f };
		Math::Vec3f rotation = { 0.f, 0.f, 0.f }; // Euler angles in degrees
		Math::Vec3f scale    = { 1.f, 1.f, 1.f };
	};
}
