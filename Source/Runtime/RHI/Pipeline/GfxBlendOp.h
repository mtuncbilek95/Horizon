#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Operation that combines the weighted source
	 * and destination values into the final color written
	 * to the render target.
	 *
	 * @code
	 *   RHI::GfxBlendState blend = {};
	 *   blend.colorOp = RHI::GfxBlendOp::Add;
	 * @endcode
	 */
	enum class GfxBlendOp : u8
	{
		Add,
		Subtract,
		ReverseSubtract,
		Min,
		Max
	};
}
