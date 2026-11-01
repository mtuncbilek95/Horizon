#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HENUM();
	enum class TextureAssetUsage : u8
	{
		Color,
		Normal,
		Mask,
		Hdr,
		UI
	};
}