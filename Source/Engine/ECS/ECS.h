#pragma once

#include <Engine/ECS/WorldRegistry.h>

namespace Horizon
{
	inline WorldRegistry& MainWorld()
	{
		static WorldRegistry instance;
		return instance;
	}
}