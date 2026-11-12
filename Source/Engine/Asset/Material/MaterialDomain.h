#pragma once

#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Asset
{
	HENUM();
	enum class MaterialDomain
	{
		Opaque,
		Masked,
		Transparent,
	};
}