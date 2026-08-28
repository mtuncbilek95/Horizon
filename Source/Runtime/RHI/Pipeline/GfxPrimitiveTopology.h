#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief How the vertex stream is assembled into
	 * primitives before rasterization.
	 *
	 * @code
	 *   RHI::GfxGraphicsPipelineDesc pipeDesc = {};
	 *   pipeDesc.topology = RHI::GfxPrimitiveTopology::TriangleList;
	 * @endcode
	 */
	enum class GfxPrimitiveTopology : u8
	{
		PointList,
		LineList,
		LineStrip,
		TriangleList,
		TriangleStrip
	};
}
