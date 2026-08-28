#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Region of the render target that clip space
	 * is mapped onto, together with the depth range
	 * written into the depth buffer.
	 *
	 * @code
	 *   RHI::GfxViewport viewport = {};
	 *   viewport.width = 1920.0f;
	 *   viewport.height = 1080.0f;
	 *   myCmdList->SetViewport(viewport);
	 * @endcode
	 */
	struct GfxViewport
	{
		f32 x = 0.0f;
		f32 y = 0.0f;
		f32 width = 0.0f;
		f32 height = 0.0f;
		f32 minDepth = 0.0f;
		f32 maxDepth = 1.0f;
	};
}