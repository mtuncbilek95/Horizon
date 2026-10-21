#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	HENUM();
	enum class MaterialParamType : u8
	{
		Float,
		Vec2,
		Vec3,
		Vec4,
		Color,
		Texture
	};
}