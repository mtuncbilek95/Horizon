#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Engine
{
	class AssetObject;

	struct AssetEntry
	{
		Guid assetId;

		std::string assetTypeName;
		Reflect::TypeHandle assetType;
		std::string cookPath;
		std::string callPath;

		AssetObject* pAssetObject;
	};
}