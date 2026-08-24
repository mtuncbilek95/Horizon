#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Pipeline/GfxBlendFactor.h>
#include <Runtime/RHI/Pipeline/GfxBlendOp.h>
#include <Runtime/RHI/Pipeline/GfxColorWrite.h>

namespace Horizon::RHI
{
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
