#pragma once

namespace Horizon
{
	struct H_EXPORT Futex
	{
		static void Wait(void* pAddress, i64 expectedValue);
		static void WakeAll(void* pAddress);
		static void WakeSingle(void* pAddress);
	};
}