#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::PAL
{
	enum class ThreadPriority : u64
	{
		Low,
		Normal,
		High,
		Highest,
		Realtime
	};
}