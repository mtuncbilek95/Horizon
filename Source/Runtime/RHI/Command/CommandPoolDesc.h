#pragma once

#include <Runtime/RHI/Queue/QueueType.h>

namespace Horizon::RHI
{
	struct CommandPoolDesc final
	{
		QueueType type = QueueType::Graphics;
	};
}
