#pragma once

#include <Runtime/Containers/String.h>

namespace Horizon
{
	struct H_EXPORT EnumValue final
	{
		String Name = {};
		i64 Value = 0;
	};
}