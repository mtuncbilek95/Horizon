#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Reflect
{
	enum class RUNTIME_API TypeMode : u8
	{
		Invalid,
		Compose,
		Array,
		Pointer
	};
}