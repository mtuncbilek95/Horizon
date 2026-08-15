#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Reflect
{
	enum class H_EXPORT TypeMode : u8
	{
		Invalid,
		Compose,
		Array,
		Pointer
	};
}