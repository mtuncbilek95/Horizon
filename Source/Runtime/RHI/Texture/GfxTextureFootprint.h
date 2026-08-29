#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	struct GfxTextureFootprint
	{
		usize rowPitch = 0;
		usize totalBytes = 0;
		u32 rowCount = 0;
		u32 depth = 1;

		b8 IsValid() const { return rowPitch > 0 && totalBytes > 0 && rowCount > 0; }
	};
}