#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::PAL
{
	enum class WatcherAction : u8 
	{
		Added,
		Removed,
		Modified,
		Renamed,
		Overflow
	};
}