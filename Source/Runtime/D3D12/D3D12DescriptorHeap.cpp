#include "D3D12DescriptorHeap.h"

#include <Runtime/Log/Terminal.h>

namespace Horizon
{
	D3D12DescriptorHeap::~D3D12DescriptorHeap()
	{
		if (m_heap)
			m_heap->Release();
	}

	u32 D3D12DescriptorHeap::Allocate()
	{
		if (!m_freeList.IsEmpty())
		{
			const u32 index = m_freeList.Back();

			m_freeList.PopBack();
			m_allocatedCount++;
			return index;
		}

		if (m_top >= m_desc.capacity)
		{
			Terminal::Error("D3D12DescriptorHeap", "Heap type {} is full, capacity {}", u32(m_desc.type), m_desc.capacity);
			return kInvalid32;
		}

		m_allocatedCount++;
		return m_top++;
	}

	void D3D12DescriptorHeap::Free(u32 index)
	{
		if (index == kInvalid32)
			return;

		m_freeList.PushBack(index);
		m_allocatedCount--;
	}
}