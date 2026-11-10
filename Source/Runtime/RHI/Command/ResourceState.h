#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class ResourceState : u32
	{
		Undefined = 0,
		Common = 1 << 0,
		VertexBuffer = 1 << 1,
		IndexBuffer = 1 << 2,
		ConstantBuffer = 1 << 3,
		IndirectArgument = 1 << 4,
		ShaderResource = 1 << 5,
		UnorderedAccess = 1 << 6,
		RenderTarget = 1 << 7,
		DepthWrite = 1 << 8,
		DepthRead = 1 << 9,
		CopySrc = 1 << 10,
		CopyDst = 1 << 11,
		Present = 1 << 12,
		ResolveSrc = 1 << 13,
		ResolveDst = 1 << 14
	};
}