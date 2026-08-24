#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class GfxQueueType : u8 
	{
		Graphics,
		Compute,
		Transfer,
		Count
	};
}