#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Rendering
{
	HENUM();
	enum class AliasingType
	{
		None,
		FXAA,
		CMAA,
	};
}