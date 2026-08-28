#pragma once

#include <Runtime/RHI/Command/GfxResourceState.h>

namespace Horizon::RHI
{
	class GfxBuffer;
	
	/**
	 * @brief A single state transition of one buffer,
	 * recorded into a command list so writes are visible
	 * to whoever reads the buffer next.
	 *
	 * @code
	 *   RHI::GfxBufferBarrier barrier = {};
	 *   barrier.pBuffer = myBuffer;
	 *   barrier.before = RHI::GfxResourceState::CopyDst;
	 *   barrier.after = RHI::GfxResourceState::ShaderResource;
	 *   myCmdList->Barrier(&barrier, 1);
	 * @endcode
	 */
	struct GfxBufferBarrier
	{
		GfxBuffer* pBuffer = nullptr;
		GfxResourceState before = GfxResourceState::Common;
		GfxResourceState after = GfxResourceState::Common;
	};
}
