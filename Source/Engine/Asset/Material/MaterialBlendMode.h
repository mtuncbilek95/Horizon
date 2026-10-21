#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HENUM();
	enum class MaterialBlendMode : u8
	{
		Opaque,
		Masked,
		Translucent
	};
}