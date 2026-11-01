#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class DescriptorType : u8
	{
		Texture,
		RWTexture,
		ConstantBuffer,
		Buffer,
		RWBuffer,
		Sampler
	};
}