#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Whether the rasterizer fills the triangles
	 * or only draws their edges.
	 *
	 * @code
	 *   RHI::GfxRasterizerState raster = {};
	 *   raster.fillMode = RHI::GfxFillMode::Wireframe;
	 * @endcode
	 */
	enum class GfxFillMode : u8
	{
		Solid,
		Wireframe
	};
}
