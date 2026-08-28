#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Which triangle facing the rasterizer throws
	 * away. The facing itself is decided by GfxFrontFace.
	 *
	 * @code
	 *   RHI::GfxRasterizerState raster = {};
	 *   raster.cullMode = RHI::GfxCullMode::Back;
	 * @endcode
	 */
	enum class GfxCullMode : u8
	{
		None,
		Front,
		Back
	};
}
