#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Pipeline/GfxCompareOp.h>
#include <Runtime/RHI/Pipeline/GfxStencilFace.h>

namespace Horizon::RHI
{
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
