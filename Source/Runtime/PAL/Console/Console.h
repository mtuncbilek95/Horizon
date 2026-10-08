#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::PAL
{
	struct RUNTIME_API Console
	{
		static b8 IsAttached();
		static b8 Hide();
		static b8 Show();
	};
}
