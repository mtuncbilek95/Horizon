#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/RTTR/Reflection.h>

#include <filesystem>
#include <string>

namespace Horizon
{
	struct H_EXPORT AssetRegistryDesc final
	{
		Guid guid = {};
		std::filesystem::path absolutePath;
		std::string assetName;
		Reflect::Type* assetType = nullptr;
		usize headerOffset = 0;
		usize sizeInBytes = 0;
	};
}