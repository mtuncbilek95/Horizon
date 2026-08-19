#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/PAL/Sync/ThreadPriority.h>

namespace Horizon::Engine
{
	enum class JobLane : u8
	{
		Critical = 0,
		Worker = 1,
		Background = 2,
		Count = 3
	};

	inline constexpr u32 LaneCount = (u32)(JobLane::Count);
	inline constexpr u32 LaneBit(JobLane lane) { return 1u << (u32)(lane); }

	inline constexpr const char* LaneName(JobLane lane)
	{
		switch (lane)
		{
		case JobLane::Critical:
			return "Critical";
		case JobLane::Worker:
			return "Worker";
		case JobLane::Background:
			return "Background";
		default:
			return "Unbound";
		}
	}

	struct H_EXPORT LaneDesc
	{
		u32 workerCount = 0;
		u32 stealMask = 0;
		u32 wakeMask = 0;
		u64 laneAffinity = 0;
		b8 exclusiveCore = false;
		b8 ecoQoS = false;
		PAL::ThreadPriority priority = PAL::ThreadPriority::Normal;
	};
}