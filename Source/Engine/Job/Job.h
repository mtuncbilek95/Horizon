#pragma once

#include <functional>
#include <atomic>

namespace Horizon
{
	using JobFunc = std::function<void()>;
	using JobRangeFunc = std::function<void(u32 begin, u32 end)>;

	struct JobCounter
	{
		std::atomic<u32> remaining = 0;

		b8 IsDone() const { return remaining.load(std::memory_order_acquire) == 0; }
	};
}
