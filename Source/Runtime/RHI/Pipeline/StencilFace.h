#pragma once

#include <Runtime/RHI/Pipeline/CompareOp.h>
#include <Runtime/RHI/Pipeline/StencilOp.h>

namespace Horizon::RHI
{
	struct StencilFace final
	{
		StencilOp failOp = StencilOp::Keep;
		StencilOp depthFailOp = StencilOp::Keep;
		StencilOp passOp = StencilOp::Keep;
		CompareOp compareOp = CompareOp::Always;
	};
}
