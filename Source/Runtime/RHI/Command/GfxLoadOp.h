#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief What happens to an attachment's existing
	 * content when a render pass begins. DontCare lets
	 * the driver discard it, which is the cheapest path.
	 *
	 * @code
	 *   RHI::GfxColorAttachment color = {};
	 *   color.loadOp = RHI::GfxLoadOp::Clear;
	 * @endcode
	 */
	enum class GfxLoadOp : u8
	{
		Load,
		Clear,
		DontCare
	};
}
