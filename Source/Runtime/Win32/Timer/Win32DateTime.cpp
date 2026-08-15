#include <Runtime/PAL/Timer/DateTime.h>

#include <Windows.h>

namespace Horizon::PAL
{
	namespace
	{
		constexpr i64 TicksFrom1601To1970 = 116444736000000000LL;
	}

	DateTime DateTime::Now()
	{
		FILETIME fileTime = {};
		GetSystemTimePreciseAsFileTime(&fileTime);

		i64 ticks = (i64)(((u64)fileTime.dwHighDateTime << 32) | (u64)fileTime.dwLowDateTime);

		return DateTime(ticks - TicksFrom1601To1970);
	}
}