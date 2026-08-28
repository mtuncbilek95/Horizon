#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Pipeline/GfxCompareOp.h>
#include <Runtime/RHI/Pipeline/GfxStencilFace.h>

namespace Horizon::RHI
{
	/**
	 * @brief Depth and stencil setup of a graphics
	 * pipeline. Depth testing and depth writing are
	 * separate, so a read only depth pass is possible.
	 *
	 * @code
	 *   RHI::GfxDepthStencilState depthStencil = {};
	 *   depthStencil.depthTest = true;
	 *   depthStencil.depthWrite = true;
	 *   depthStencil.depthCompare = RHI::GfxCompareOp::Less;
	 * @endcode
	 */
	struct GfxDepthStencilState
	{
		b8 depthTest = false;
		b8 depthWrite = false;
		GfxCompareOp depthCompare = GfxCompareOp::Less;

		b8 stencilTest = false;
		u8 stencilReadMask = 0xFF;
		u8 stencilWriteMask = 0xFF;

		GfxStencilFace front;
		GfxStencilFace back;
	};
}
