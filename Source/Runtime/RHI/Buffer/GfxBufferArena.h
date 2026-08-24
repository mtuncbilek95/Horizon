#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Buffer/GfxBufferArenaDesc.h>
#include <Runtime/RHI/Buffer/GfxBufferRange.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon::RHI
{
	class GfxBufferArena : public GfxObject
	{
	public:
		virtual GfxBufferRange Allocate(usize size, usize alignment = 16) = 0;
		virtual void Free(const GfxBufferRange& range) = 0;
		virtual void Reset() = 0;

		const GfxBufferArenaDesc& GetDesc() const { return m_desc; }
		GfxBuffer* GetBuffer() const { return m_buffer; }
		usize GetCapacity() const { return m_desc.capacity; }
		usize GetUsedBytes() const { return m_usedBytes; }

	protected:
		GfxBufferArenaDesc m_desc{};
		GfxBuffer* m_buffer = nullptr;
		usize m_usedBytes = 0;
	};
}
