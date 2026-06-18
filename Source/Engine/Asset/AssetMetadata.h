#pragma once

#include <Runtime/Containers/Guid.h>
#include <Engine/Asset/AssetType.h>

#include <vector>
#include <string>

namespace Horizon
{
	struct AssetMetadata
	{
		Guid guid;
		AssetTypeId type = AssetTypeId::Invalid;
		std::string cookedPath;

		std::vector<Guid> dependencies;
	};
}