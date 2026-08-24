#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	class GfxBuffer;

	struct GfxBufferRange
	{
		GfxBuffer* pBuffer = nullptr;
		usize offset = 0;
		usize size = 0;
		u64 allocationHandle = kInvalid64;

		b8 IsValid() const { return pBuffer != nullptr && size > 0; }
	};
}
