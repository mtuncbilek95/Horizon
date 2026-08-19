#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::PAL
{
	enum class WatcherEntryKind : u8
	{
		Unknown,
		File,
		Directory
	};
}