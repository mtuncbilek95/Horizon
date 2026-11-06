#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon
{
	struct Range final
	{
		u64 offset = kInvalid64;
		u64 size = 0;

		b8 IsValid() const { return offset != kInvalid64; }
	};

	class RangeAllocator final
	{
	public:
		void Initialize(u64 capacity);
		void Reset();

		Range Allocate(u64 size, u64 alignment);
		void Free(const Range& range);

		u64 GetCapacity() const { return m_capacity; }

	private:
		List<Range> m_freeBlocks;
		u64 m_capacity = 0;
	};
}