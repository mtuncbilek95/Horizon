#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::PAL
{
	struct H_EXPORT Futex
	{
		static void Wait(void* pAddress, i64 expectedValue);
		static void WakeAll(void* pAddress);
		static void WakeSingle(void* pAddress);
	};
}