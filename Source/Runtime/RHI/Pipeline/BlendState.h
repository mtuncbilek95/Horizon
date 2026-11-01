#pragma once

#include <Runtime/RHI/Pipeline/BlendFactor.h>
#include <Runtime/RHI/Pipeline/BlendOp.h>
#include <Runtime/RHI/Pipeline/ColorWrite.h>

namespace Horizon::RHI
{
	struct BlendState final
	{
		b8 enable = false;

		BlendFactor srcColor = BlendFactor::One;
		BlendFactor dstColor = BlendFactor::Zero;
		BlendOp colorOp = BlendOp::Add;

		BlendFactor srcAlpha = BlendFactor::One;
		BlendFactor dstAlpha = BlendFactor::Zero;
		BlendOp alphaOp = BlendOp::Add;

		ColorWrite writeMask = ColorWrite::All;
	};
}
