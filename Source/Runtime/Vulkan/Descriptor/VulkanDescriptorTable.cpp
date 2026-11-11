#include "VulkanDescriptorTable.h"

#include <Runtime/Vulkan/Descriptor/VulkanDescriptorHeap.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::RHI
{
	VulkanDescriptorTable::~VulkanDescriptorTable()
	{
		if (m_count > 0)
			static_cast<VulkanDescriptorHeap*>(m_heap)->FreeRange(m_offset, m_count);
	}

	void VulkanDescriptorTable::Write(u32 binding, u32 arrayIndex, TextureView* pView)
	{
		const Slot* pSlot = FindSlot(binding, arrayIndex);
		if (!pSlot)
			return;

		m_heap->Write(m_offset + pSlot->offset + arrayIndex, pSlot->type, pView);
	}

	void VulkanDescriptorTable::Write(u32 binding, u32 arrayIndex, Buffer* pBuffer, u64 offset, u64 size, u32 stride)
	{
		const Slot* pSlot = FindSlot(binding, arrayIndex);
		if (!pSlot)
			return;

		m_heap->Write(m_offset + pSlot->offset + arrayIndex, pSlot->type, pBuffer, offset, size, stride);
	}

	void VulkanDescriptorTable::Write(u32 binding, u32 arrayIndex, Sampler* pSampler)
	{
		const Slot* pSlot = FindSlot(binding, arrayIndex);
		if (!pSlot)
			return;

		m_heap->Write(m_offset + pSlot->offset + arrayIndex, pSampler);
	}

	u64 VulkanDescriptorTable::GpuHandle() const
	{
		return static_cast<VulkanDescriptorHeap*>(m_heap)->GpuAddress();
	}

	const VulkanDescriptorTable::Slot* VulkanDescriptorTable::FindSlot(u32 binding, u32 arrayIndex) const
	{
		if (binding >= m_slots.GetCount())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Binding {} is not in table {}", binding, m_tableIndex);
			return nullptr;
		}

		const Slot& slot = m_slots[binding];

		if (arrayIndex >= slot.count)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Binding {} index {} is out of range {}", binding, arrayIndex, slot.count);
			return nullptr;
		}
		return &slot;
	}
}