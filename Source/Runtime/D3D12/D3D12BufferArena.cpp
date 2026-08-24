#include "D3D12BufferArena.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

#include <Runtime/D3D12/D3D12Buffer.h>

namespace Horizon::RHI
{
	D3D12BufferArena::~D3D12BufferArena()
	{
		if (m_block)
		{
			m_block->Clear();
			m_block->Release();
		}

		if (m_buffer)
			Memory::Allocator::Delete(m_buffer);
	}

	GfxBufferRange D3D12BufferArena::Allocate(usize size, usize alignment)
	{
		if (size == 0 || size > m_desc.capacity)
		{
			Terminal::Error(StringOps::GetName(this), "{} byte request does not fit the {} byte arena", size, m_desc.capacity);
			return {};
		}

		D3D12MA::VIRTUAL_ALLOCATION_DESC allocDesc = {};

		allocDesc.Size = size;
		allocDesc.Alignment = alignment;

		D3D12MA::VirtualAllocation allocation = {};
		u64 offset = 0;

		HRESULT hr = m_block->Allocate(&allocDesc, &allocation, &offset);

		if (FAILED(hr))
		{
			Terminal::Error(StringOps::GetName(this), "Arena is out of space, {} bytes requested with {} in use", size, m_usedBytes);
			return {};
		}

		m_usedBytes += size;

		GfxBufferRange range = {};

		range.pBuffer = m_buffer;
		range.offset = usize(offset);
		range.size = size;
		range.allocationHandle = allocation.AllocHandle;

		return range;
	}

	void D3D12BufferArena::Free(const GfxBufferRange& range)
	{
		if (!range.IsValid() || range.allocationHandle == kInvalid64)
			return;

		D3D12MA::VirtualAllocation allocation = {};

		allocation.AllocHandle = range.allocationHandle;

		m_block->FreeAllocation(allocation);
		m_usedBytes -= range.size;
	}

	void D3D12BufferArena::Reset()
	{
		m_block->Clear();
		m_usedBytes = 0;
	}
}
