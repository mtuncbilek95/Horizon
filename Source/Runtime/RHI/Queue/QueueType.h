#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class QueueType : u8
	{
		Graphics,
		Compute,
		Transfer
	};
}