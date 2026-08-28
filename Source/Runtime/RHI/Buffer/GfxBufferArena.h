#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Buffer/GfxBufferArenaDesc.h>
#include <Runtime/RHI/Buffer/GfxBufferRange.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon::RHI
{
	/**
	 * @brief Sub allocator over a single large GfxBuffer.
	 * Hands out GfxBufferRange windows so that many small
	 * uploads can share one device allocation.
	 *
	 * @code
	 *   RHI::GfxBufferArenaDesc arenaDesc = {};
	 *   RHI::GfxBufferArena* arena = nullptr;
	 *   arena = myRhiDevice->CreateBufferArena(arenaDesc);
	 *   RHI::GfxBufferRange range = arena->Allocate(1024);
	 * @endcode
	 */
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
