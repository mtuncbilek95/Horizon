#pragma once

#include <Runtime/RHI/Pipeline/GfxCompareOp.h>
#include <Runtime/RHI/Pipeline/GfxStencilOp.h>

namespace Horizon::RHI
{
	struct GfxStencilFace
	{
		GfxStencilOp failOp = GfxStencilOp::Keep;
		GfxStencilOp depthFailOp = GfxStencilOp::Keep;
		GfxStencilOp passOp = GfxStencilOp::Keep;
		GfxCompareOp compareOp = GfxCompareOp::Always;
	};
}
