#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <string>

namespace Horizon::Reflect
{
	struct RUNTIME_API EnumValue
	{
		std::string name;
		i64 value = 0;
	};
}