#include "D3D12UploadRing.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

#include <Runtime/D3D12/D3D12Buffer.h>

#include <Runtime/RHI/Fence/GfxFence.h>

namespace Horizon::RHI
{
	namespace
	{
		u64 AlignUp(u64 value, u64 alignment)
		{
			if (alignment == 0)
				return value;

			return (value + alignment - 1) & ~(alignment - 1);
		}
	}

	D3D12UploadRing::~D3D12UploadRing()
	{
		if (m_buffer)
			Memory::Allocator::Delete(m_buffer);
	}

	b8 D3D12UploadRing::RetireOldest()
	{
		if (m_markers.IsEmpty())
			return false;

		const Marker marker = m_markers.Front();

		if (!m_fence)
		{
			Terminal::Error(StringOps::GetName(this), "Ring has markers but no fence to wait on");
			return false;
		}

		m_fence->WaitCPU(marker.fenceValue);

		m_tail = marker.head;
		m_markers.PopFront();

		return true;
	}

	GfxUploadRange D3D12UploadRing::Allocate(usize size, usize alignment)
	{
		if (size == 0 || size > m_desc.capacity)
		{
			Terminal::Error(StringOps::GetName(this), "{} byte request does not fit the {} byte ring", size, m_desc.capacity);
			return {};
		}

		u64 head = AlignUp(m_head, alignment);
		usize offset = usize(head % m_desc.capacity);

		if (offset + size > m_desc.capacity)
		{
			head += m_desc.capacity - offset;
			offset = 0;
		}

		while ((head + size) - m_tail > m_desc.capacity)
		{
			if (!RetireOldest())
			{
				Terminal::Error(StringOps::GetName(this), "Ring is exhausted, {} bytes requested with {} in flight", size, m_usedBytes);
				return {};
			}
		}

		m_head = head + size;
		m_usedBytes = usize(m_head - m_tail);

		GfxUploadRange range = {};

		range.pBuffer = m_buffer;
		range.offset = offset;
		range.size = size;
		range.pMapped = m_mapped + offset;

		return range;
	}

	void D3D12UploadRing::MarkFrame(GfxFence* pFence, u64 fenceValue)
	{
		m_fence = pFence;

		Marker marker = {};

		marker.head = m_head;
		marker.fenceValue = fenceValue;

		m_markers.PushBack(marker);
	}
}
