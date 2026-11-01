#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class TextureUsage : u32
	{
		None = 0,
		Sampled = 1 << 0,
		Storage = 1 << 1,
		RenderTarget = 1 << 2,
		DepthStencil = 1 << 3,
		CopySrc = 1 << 4,
		CopyDst = 1 << 5
	};
}