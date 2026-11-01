#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class SamplerAddressMode : u8
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder,
		MirrorClampToEdge
	};
}
