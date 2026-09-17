#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	enum class AssetResidency : u8 
	{
		Unloaded,
		Reading,
		Decoding,
		Uploading,
		Resident,
		Failed
	};
}