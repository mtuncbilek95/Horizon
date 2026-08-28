#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	class GfxBuffer;

	/**
	 * @brief A staging window handed out by a
	 * GfxUploadRing. It stays mapped, so the payload is
	 * written straight through pMapped and then copied to
	 * its device local destination.
	 *
	 * @code
	 *   RHI::GfxUploadRange range = myUploadRing->Allocate(size);
	 *   memcpy(range.pMapped, pSource, size);
	 *   myCmdList->CopyBuffer(range.pBuffer, range.offset, myBuffer, 0, size);
	 * @endcode
	 */
	struct GfxUploadRange
	{
		GfxBuffer* pBuffer = nullptr;
		usize offset = 0;
		usize size = 0;
		void* pMapped = nullptr;

		b8 IsValid() const { return pBuffer != nullptr && pMapped != nullptr && size > 0; }
	};
}
