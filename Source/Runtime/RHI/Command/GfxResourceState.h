#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class GfxResourceState : u32
	{
		Common = 0,
		VertexBuffer = 1 << 0,
		IndexBuffer = 1 << 1,
		ConstantBuffer = 1 << 2,
		IndirectArgument = 1 << 3,
		ShaderResource = 1 << 4,
		UnorderedAccess = 1 << 5,
		RenderTarget = 1 << 6,
		DepthWrite = 1 << 7,
		DepthRead = 1 << 8,
		CopySrc = 1 << 9,
		CopyDst = 1 << 10,

		Present = 1 << 11
	};
}
