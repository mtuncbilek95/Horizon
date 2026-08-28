#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Hardware queue family a command list is
	 * recorded for. A list may only be submitted to a
	 * queue of a matching type.
	 *
	 * @code
	 *   RHI::GfxCommandList* cmdList = nullptr;
	 *   cmdList = myRhiDevice->CreateCommandList(RHI::GfxQueueType::Transfer);
	 * @endcode
	 */
	enum class GfxQueueType : u8 
	{
		Graphics,
		Compute,
		Transfer,
		Count
	};
}