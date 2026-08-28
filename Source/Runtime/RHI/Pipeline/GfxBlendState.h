#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Pipeline/GfxBlendFactor.h>
#include <Runtime/RHI/Pipeline/GfxBlendOp.h>
#include <Runtime/RHI/Pipeline/GfxColorWrite.h>

namespace Horizon::RHI
{
	/**
	 * @brief Blending setup of a single render target,
	 * with independent factors for the color and the alpha
	 * channels plus the write mask.
	 *
	 * @code
	 *   RHI::GfxBlendState blend = {};
	 *   blend.enable = true;
	 *   blend.srcColor = RHI::GfxBlendFactor::SrcAlpha;
	 *   blend.dstColor = RHI::GfxBlendFactor::InvSrcAlpha;
	 * @endcode
	 */
	struct GfxBlendState
	{
		b8 enable = false;

		GfxBlendFactor srcColor = GfxBlendFactor::One;
		GfxBlendFactor dstColor = GfxBlendFactor::Zero;
		GfxBlendOp colorOp = GfxBlendOp::Add;

		GfxBlendFactor srcAlpha = GfxBlendFactor::One;
		GfxBlendFactor dstAlpha = GfxBlendFactor::Zero;
		GfxBlendOp alphaOp = GfxBlendOp::Add;

		GfxColorWrite writeMask = GfxColorWrite::All;
	};
}
