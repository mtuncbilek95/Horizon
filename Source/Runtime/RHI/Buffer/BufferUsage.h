#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class BufferUsage : u32
	{
		None = 0,
		Vertex = 1 << 0,
		Index = 1 << 1,
		Constant = 1 << 2,
		Storage = 1 << 3,
		Indirect = 1 << 4,
		CopySrc = 1 << 5,
		CopyDst = 1 << 6
	};
}