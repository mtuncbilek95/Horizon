#include <Runtime/PAL/Sync/Atomic.h>

#include <Windows.h>

namespace Horizon::PAL
{
	i64 AtomicOps::Load(const i64* pValue, MemoryOrder)
	{
		return InterlockedCompareExchange64((volatile LONG64*)pValue, 0, 0);
	}

	i64 AtomicOps::FetchStore(i64* pTarget, const i64 value, MemoryOrder)
	{
		return InterlockedExchange64((volatile LONG64*)pTarget, value);
	}

	i64 AtomicOps::Exchange(i64* pTarget, const i64 value, MemoryOrder)
	{
		return InterlockedExchange64((volatile LONG64*)pTarget, value);
	}

	i64 AtomicOps::FetchAdd(i64* pTarget, const i64 value, MemoryOrder)
	{
		return InterlockedExchangeAdd64((volatile LONG64*)pTarget, value);
	}

	i64 AtomicOps::FetchSubtract(i64* pTarget, const i64 value, MemoryOrder)
	{
		return InterlockedExchangeAdd64((volatile LONG64*)pTarget, -value);
	}

	i64 AtomicOps::Add(i64* pTarget, const i64 value, MemoryOrder)
	{
		return InterlockedExchangeAdd64((volatile LONG64*)pTarget, value) + value;
	}

	i64 AtomicOps::Subtract(i64* pTarget, const i64 value, MemoryOrder)
	{
		return InterlockedExchangeAdd64((volatile LONG64*)pTarget, -value) - value;
	}

	i64 AtomicOps::CompareExchange(i64* pTarget, const i64 expectedValue, const i64 desiredValue, MemoryOrder)
	{
		return InterlockedCompareExchange64((volatile LONG64*)pTarget, desiredValue, expectedValue);
	}
}