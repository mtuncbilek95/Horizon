#pragma once

#include <Runtime/RHI/Command/GfxResourceState.h>

namespace Horizon::RHI
{
	class GfxTexture;

	struct GfxTextureBarrier
	{
		GfxTexture* pTexture = nullptr;
		GfxResourceState before = GfxResourceState::Common;
		GfxResourceState after = GfxResourceState::Common;
	};
}
