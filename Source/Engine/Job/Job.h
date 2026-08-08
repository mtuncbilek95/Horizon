#pragma once

#include <Runtime/PAL/Sync/Atomic.h>

#include <functional>

namespace Horizon::Engine
{
	using Job = std::function<void()>;

	struct H_EXPORT JobCounter
	{
		PAL::Atomic<u32> remaining = 0;
	};
}