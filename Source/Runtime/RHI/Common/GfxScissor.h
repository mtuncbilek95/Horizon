#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Pixel rectangle that clips rasterized
	 * fragments. Anything outside the rectangle is
	 * discarded before the pixel shader runs.
	 *
	 * @code
	 *   RHI::GfxScissor scissor = { 0, 0, 1920, 1080 };
	 *   myCmdList->SetScissor(scissor);
	 * @endcode
	 */
	struct GfxScissor
	{
		i32 x = 0;
		i32 y = 0;
		i32 width = 0;
		i32 height = 0;
	};
}