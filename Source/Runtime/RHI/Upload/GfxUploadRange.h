#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	class GfxBuffer;

	inline constexpr usize GfxUploadBufferAlignment = 256;
	inline constexpr usize GfxUploadTextureAlignment = 512;

	struct GfxUploadRange
	{
		GfxBuffer* pBuffer = nullptr;
		usize offset = 0;
		usize size = 0;
		void* pMapped = nullptr;

		b8 IsValid() const { return pBuffer != nullptr && pMapped != nullptr && size > 0; }
	};
}
