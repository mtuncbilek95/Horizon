#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HENUM();
	enum class TextureAssetUsage : u8
	{
		Color,
		Normal,
		Mask,
	};
}