#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	struct ScriptProperties
	{
		static constexpr usize MaxChunkName = 128;

		u64 sourceSize = 0;
		c8 chunkName[MaxChunkName] = {};
	};
}