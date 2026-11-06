#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <string>

namespace Horizon::Reflect
{
	struct EnumValue
	{
		std::string name;
		i64 value = 0;
	};
}