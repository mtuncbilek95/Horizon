#pragma once

#include <Runtime/RHI/Pipeline/GfxCompareOp.h>
#include <Runtime/RHI/Pipeline/GfxStencilOp.h>

namespace Horizon::RHI
{
	/**
	 * @brief Stencil behaviour of one triangle facing.
	 * Each of the three outcomes of the test gets its own
	 * operation.
	 *
	 * @code
	 *   RHI::GfxStencilFace face = {};
	 *   face.compareOp = RHI::GfxCompareOp::Equal;
	 *   face.passOp = RHI::GfxStencilOp::Replace;
	 * @endcode
	 */
	struct GfxStencilFace
	{
		GfxStencilOp failOp = GfxStencilOp::Keep;
		GfxStencilOp depthFailOp = GfxStencilOp::Keep;
		GfxStencilOp passOp = GfxStencilOp::Keep;
		GfxCompareOp compareOp = GfxCompareOp::Always;
	};
}
