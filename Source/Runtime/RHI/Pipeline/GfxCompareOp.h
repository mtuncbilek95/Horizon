#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Comparison function of the depth test, the
	 * stencil test and the comparison samplers. The new
	 * value is the left hand side of the comparison.
	 *
	 * @code
	 *   RHI::GfxDepthStencilState depthStencil = {};
	 *   depthStencil.depthCompare = RHI::GfxCompareOp::LessEqual;
	 * @endcode
	 */
	enum class GfxCompareOp : u8
	{
		Never,
		Less,
		Equal,
		LessEqual,
		Greater,
		NotEqual,
		GreaterEqual,
		Always
	};
}
