#pragma once

#include <Runtime/PAL/Sync/Atomic.h>

#include <functional>

namespace Horizon
{
	using Job = std::function<void()>;

	struct JobCounter
	{
		PAL::Atomic<u32> remaining = 0;
	};
}