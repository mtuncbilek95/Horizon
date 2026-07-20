#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <string>
#include <vector>

namespace Horizon
{
	struct ProducedAsset
	{
		std::string subName;
		std::string assetTypeName;
		Guid guid;
		std::vector<u8> cooked;
	};
}