#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Containers/List.h>

namespace Horizon::PAL
{
	struct H_EXPORT CoreInfo
	{
		u32 logicalIndex;
		u32 physicalIndex;
		u8 efficiencyClass;
		b8 isPerformance;
		b8 isPrimarySibling;
	};

	struct H_EXPORT Processor
	{
		static List<CoreInfo> EnumerateCores();
		static u32 PerformanceCoreCount();
		static u32 EfficiencyCoreCount();
		static u32 PhysicalCoreCount();
	};
}