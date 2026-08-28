#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Winding order that counts as the front face
	 * of a triangle, which in turn drives culling.
	 *
	 * @code
	 *   RHI::GfxRasterizerState raster = {};
	 *   raster.frontFace = RHI::GfxFrontFace::CCW;
	 * @endcode
	 */
	enum class GfxFrontFace : u8
	{
		CW,
		CCW
	};
}
