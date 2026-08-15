#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Definitions/BitwiseOperators.h>

namespace Horizon::Engine
{
	enum class AssetOrigin : u8
	{
		Imported,
		Native,
		Generated
	};
}