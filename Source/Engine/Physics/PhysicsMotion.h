#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HENUM();
	enum class PhysicsMotion
	{
		Static,
		Dynamic,
		Kinematic
	};
}