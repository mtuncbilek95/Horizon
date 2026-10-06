#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RHI/Queue/GfxQueueType.h>

namespace Horizon::RHI
{
	struct GfxQueryHeapDesc
	{
		GfxQueueType queue = GfxQueueType::Graphics;
		u32 count = 0;
	};
}
