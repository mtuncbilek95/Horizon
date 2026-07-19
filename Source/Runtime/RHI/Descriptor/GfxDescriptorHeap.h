#pragma once

#include <Runtime/RHI/GfxTypes.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon
{
	struct GfxDescriptorHeapDesc
	{
		GfxDescriptorHeapType type = GfxDescriptorHeapType::Resource;
		u32 capacity = 0;
		b8 shaderVisible = false;
	};

	class GfxDescriptorHeap : public GfxObject
	{
	public:
		virtual u32 Allocate() = 0;
		virtual void Free(u32 index) = 0;

		const GfxDescriptorHeapDesc& GetDesc() const { return m_desc; }
		u32 GetCapacity() const { return m_desc.capacity; }
		u32 GetAllocatedCount() const { return m_allocatedCount; }

	protected:
		GfxDescriptorHeapDesc m_desc{};
		u32 m_allocatedCount = 0;
	};
}