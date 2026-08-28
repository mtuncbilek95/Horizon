#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Whether an attachment's content is written
	 * back to memory when a render pass ends. DontCare
	 * is meant for transient targets nothing reads later.
	 *
	 * @code
	 *   RHI::GfxDepthAttachment depth = {};
	 *   depth.storeOp = RHI::GfxStoreOp::DontCare;
	 * @endcode
	 */
	enum class GfxStoreOp : u8
	{
		Store,
		DontCare
	};
}
