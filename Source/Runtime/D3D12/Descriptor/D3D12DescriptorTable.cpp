#include "D3D12DescriptorTable.h"

#include <Runtime/D3D12/Descriptor/D3D12DescriptorHeap.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::RHI
{
	D3D12DescriptorTable::~D3D12DescriptorTable()
	{
		if (m_count > 0)
			static_cast<D3D12DescriptorHeap*>(m_heap)->FreeRange(m_offset, m_count);
	}

	void D3D12DescriptorTable::Write(u32 binding, u32 arrayIndex, TextureView* pView)
	{
		const Slot* pSlot = FindSlot(binding, arrayIndex);
		if (!pSlot)
			return;

		m_heap->Write(m_offset + pSlot->offset + arrayIndex, pSlot->type, pView);
	}

	void D3D12DescriptorTable::Write(u32 binding, u32 arrayIndex, Buffer* pBuffer, u64 offset, u64 size, u32 stride)
	{
		const Slot* pSlot = FindSlot(binding, arrayIndex);
		if (!pSlot)
			return;

		m_heap->Write(m_offset + pSlot->offset + arrayIndex, pSlot->type, pBuffer, offset, size, stride);
	}

	void D3D12DescriptorTable::Write(u32 binding, u32 arrayIndex, Sampler* pSampler)
	{
		const Slot* pSlot = FindSlot(binding, arrayIndex);
		if (!pSlot)
			return;

		m_heap->Write(m_offset + pSlot->offset + arrayIndex, pSampler);
	}

	D3D12_GPU_DESCRIPTOR_HANDLE D3D12DescriptorTable::GpuHandle() const
	{
		return static_cast<D3D12DescriptorHeap*>(m_heap)->GpuAt(m_offset);
	}

	const D3D12DescriptorTable::Slot* D3D12DescriptorTable::FindSlot(u32 binding, u32 arrayIndex) const
	{
		for (const Slot& slot : m_slots)
		{
			if (slot.binding != binding)
				continue;

			if (arrayIndex >= slot.count)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "Binding {} index {} is out of range {}", binding, arrayIndex, slot.count);
				return nullptr;
			}

			return &slot;
		}

		Terminal::Error(StringHelper::GetTypeName(this), "Binding {} is not in table {}", binding, m_tableIndex);
		return nullptr;
	}
}