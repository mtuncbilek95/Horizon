#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class GfxDescriptorHeapType : u8
	{
		Resource,
		Sampler,
		Color,
		Depth
	};
}