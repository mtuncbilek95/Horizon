#pragma once

#include <Runtime/RHI/Queue/QueuePriority.h>
#include <Runtime/RHI/Queue/QueueType.h>

namespace Horizon::RHI
{
	struct QueueDesc
	{
		QueueType type;
		QueuePriority priority;
	};
}