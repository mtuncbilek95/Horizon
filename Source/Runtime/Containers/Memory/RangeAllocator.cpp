#include "RangeAllocator.h"

namespace Horizon
{
	void RangeAllocator::Initialize(u64 capacity)
	{
		m_capacity = capacity;
		Reset();
	}

	void RangeAllocator::Reset()
	{
		m_freeBlocks.Clear();
		m_freeBlocks.PushBack(Range{ 0, m_capacity });
	}

	Range RangeAllocator::Allocate(u64 size, u64 alignment)
	{
		if (alignment == 0)
			alignment = 1;

		for (usize i = 0; i < m_freeBlocks.GetCount(); i++)
		{
			Range block = m_freeBlocks[i];
			u64 alignedOffset = (block.offset + alignment - 1) / alignment * alignment;
			u64 padding = alignedOffset - block.offset;

			if (padding + size > block.size)
				continue;

			u64 tailOffset = alignedOffset + size;
			u64 tailSize = block.offset + block.size - tailOffset;

			m_freeBlocks.RemoveAt(i);

			if (tailSize > 0)
				m_freeBlocks.PushAt(i, Range{ tailOffset, tailSize });

			if (padding > 0)
				m_freeBlocks.PushAt(i, Range{ block.offset, padding });

			return Range{ alignedOffset, size };
		}

		return Range();
	}

	void RangeAllocator::Free(const Range& range)
	{
		if (!range.IsValid() || range.size == 0)
			return;

		usize index = 0;
		while (index < m_freeBlocks.GetCount() && m_freeBlocks[index].offset < range.offset)
			index++;

		m_freeBlocks.PushAt(index, range);

		if (index + 1 < m_freeBlocks.GetCount())
		{
			Range& curr = m_freeBlocks[index];
			const Range& next = m_freeBlocks[index + 1];

			if (curr.offset + curr.size == next.offset)
			{
				curr.size += next.size;
				m_freeBlocks.RemoveAt(index + 1);
			}
		}

		if (index > 0)
		{
			Range& prev = m_freeBlocks[index - 1];
			const Range& curr = m_freeBlocks[index];

			if (prev.offset + prev.size == curr.offset)
			{
				prev.size += curr.size;
				m_freeBlocks.RemoveAt(index);
			}
		}
	}
}