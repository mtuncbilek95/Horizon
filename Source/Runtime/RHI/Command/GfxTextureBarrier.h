#pragma once

#include <Runtime/RHI/Command/GfxResourceState.h>

namespace Horizon::RHI
{
	class GfxTexture;

	 /**
	  * @brief A single state transition of one texture,
	  * recorded into a command list so the GPU can flush
	  * and lay the resource out for its next usage.
	  *
	  * @code
	  *   RHI::GfxTextureBarrier barrier = {};
	  *   barrier.pTexture = myTexture;
	  *   barrier.before = RHI::GfxResourceState::Common;
	  *   barrier.after = RHI::GfxResourceState::RenderTarget;
	  *   myCmdList->Barrier(&barrier, 1);
	  * @endcode
	  */
	struct GfxTextureBarrier
	{
		GfxTexture* pTexture = nullptr;
		GfxResourceState before = GfxResourceState::Common;
		GfxResourceState after = GfxResourceState::Common;
	};
}
