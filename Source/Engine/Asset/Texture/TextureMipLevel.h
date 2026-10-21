#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HENUM();
	enum class TextureMipLevel : u8
	{
		Mip0, // 4K
		Mip1, // 2K
		Mip2, // 1K
		Mip3, // 512
		Mip4, // 256
		Mip5, // 128
	};
}