#pragma once

#include <Runtime/Containers/List.h>
#include <Runtime/PAL/Sync/Atomic.h>

namespace Horizon::Engine
{
	class JobWorker;

	struct Lane
	{
		List<JobWorker*> laneWorkers;
		PAL::Atomic<usize> next = 0;
	};
}