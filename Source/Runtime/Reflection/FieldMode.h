#pragma once

#include <Runtime/Reflection/Definitions.h>

namespace Horizon
{
	HENUM();
	enum class FieldMode : i64
	{
		Normal,
		Object,
		Array
	};
}