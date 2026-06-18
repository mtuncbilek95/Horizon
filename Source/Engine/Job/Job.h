#pragma once

#include <functional>
#include <atomic>

namespace Horizon
{
	using Job = std::function<void()>;

	struct JobCounter
	{
		std::atomic<u32> remaining = 0;
	};
}
