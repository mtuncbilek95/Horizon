#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <atomic>

namespace Horizon
{
	inline u32 NextTypeId()
	{
		static u32 counter = 0;
		return counter++;
	}

	template<typename T>
	u32 TypeId()
	{
		static const u32 id = NextTypeId();
		return id;
	}

	inline u32 NextAtomicTypeId()
	{
		static std::atomic<u32> counter{ 0 };
		return counter.fetch_add(1, std::memory_order_relaxed);
	}

	template<typename T>
	u32 AtomicTypeId()
	{
		static const u32 id = NextAtomicTypeId();
		return id;
	}
}
