#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HENUM();
	enum class ColliderShape
	{
		Box,
		Sphere,
		Capsule,
		Cylinder
	};
}