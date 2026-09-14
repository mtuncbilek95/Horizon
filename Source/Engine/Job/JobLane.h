#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	enum class JobLane : u8
	{
		Critical = 0,
		Background = 1
	};
}