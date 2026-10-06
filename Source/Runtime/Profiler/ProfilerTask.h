#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Profiler
{
	struct H_EXPORT Task
	{
		const c8* pName = nullptr;
		f64 startMs = 0.0;
		f64 endMs = 0.0;
		u32 color = 0;
		u32 depth = 0;
	};
}