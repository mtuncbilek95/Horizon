#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	class GfxBuffer;

	/**
	 * @brief A sub allocation inside a GfxBuffer, handed
	 * out by a GfxBufferArena. Carries the owning buffer,
	 * the byte window and the handle needed to free it.
	 *
	 * @code
	 *   RHI::GfxBufferRange range = myArena->Allocate(1024);
	 *   if (range.IsValid())
	 *       myArena->Free(range);
	 * @endcode
	 */
	struct GfxBufferRange
	{
		GfxBuffer* pBuffer = nullptr;
		usize offset = 0;
		usize size = 0;
		u64 allocationHandle = kInvalid64;

		b8 IsValid() const { return pBuffer != nullptr && size > 0; }
	};
}
