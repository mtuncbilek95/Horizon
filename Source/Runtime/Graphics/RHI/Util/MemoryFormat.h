#pragma once

namespace Horizon
{
	enum class MemoryUsage
	{
		Unknown = 0,
		GpuLazy = 1 << 0,
		Auto = 1 << 1,
		AutoPreferDevice = 1 << 2,
		AutoPreferHost = 1 << 3
	};

	enum class MemoryAllocation
	{
		DedicatedMemory = 1 << 0,
		NeverAllocate = 1 << 1,
		Mapped = 1 << 2,
		UpperAddress = 1 << 3,
		DontBind = 1 << 4,
		WithinBudget = 1 << 5,
		CanAlias = 1 << 6,
		SequentialWrite = 1 << 7,
		HostAccessRandom = 1 << 8,
		AllowTransferInstead = 1 << 9,
		StrategyMinMemory = 1 << 10,
		StrategyMinTime = 1 << 11,
		StrategyMinOffset = 1 << 12,
		StrategyMask = StrategyMinMemory | 
		StrategyMinTime | StrategyMinOffset
	};
}
