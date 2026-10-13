#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	namespace SystemPhase
	{
		constexpr u32 Script = 1000;
		constexpr u32 Physics = 2000;
		constexpr u32 Transform = 3000;
		constexpr u32 Camera = 4000;
		constexpr u32 Mesh = 9000;
	}
}