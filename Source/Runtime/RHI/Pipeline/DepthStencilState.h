#pragma once

#include <Runtime/RHI/Pipeline/StencilFace.h>

namespace Horizon::RHI
{
	struct DepthStencilState final
	{
		b8 depthTest = false;
		b8 depthWrite = false;
		CompareOp depthCompare = CompareOp::Less;

		b8 stencilTest = false;
		u8 stencilReadMask = 0xFF;
		u8 stencilWriteMask = 0xFF;

		StencilFace front;
		StencilFace back;
	};
}
