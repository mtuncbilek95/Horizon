#pragma once

#include <Runtime/RHI/Command/GfxResourceState.h>

namespace Horizon::RHI
{
	class GfxTexture;

	inline constexpr u32 GfxAllMips = ~0u;
	inline constexpr u32 GfxAllSlices = ~0u;

	struct GfxTextureBarrier
	{
		GfxTexture* pTexture = nullptr;
		GfxResourceState before = GfxResourceState::Common;
		GfxResourceState after = GfxResourceState::Common;
		u32 firstMip = 0;
		u32 mipCount = GfxAllMips;
		u32 firstSlice = 0;
		u32 sliceCount = GfxAllSlices;
	};
}
