#pragma once

#include <Engine/Job/JobLane.h>

#include <Runtime/PAL/Sync/Atomic.h>

#include <functional>

namespace Horizon::Engine
{
	using Job = std::function<void()>;
	using ParallelJob = std::function<void(usize, usize)>;

	struct H_EXPORT JobCounter
	{
		PAL::Atomic<u32> remaining = 0;
		PAL::Atomic<u32> lane = (u32)(JobLane::Count);
	};
}