#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Asset
{
	HENUM();
	enum class TextureUsage : u8
	{
		Color,
		Normal,
		Mask,
		Hdr,
		UI
	};
}