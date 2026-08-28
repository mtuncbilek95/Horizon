#pragma once

#include <Runtime/RHI/Command/GfxResourceState.h>

namespace Horizon::RHI
{
	class GfxBuffer;
	
	struct GfxBufferBarrier
	{
		GfxBuffer* pBuffer = nullptr;
		GfxResourceState before = GfxResourceState::Common;
		GfxResourceState after = GfxResourceState::Common;
	};
}
