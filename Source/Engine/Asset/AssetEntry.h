#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Engine
{
	struct AssetEntry
	{
		Guid assetId;
		std::string cookedPath;

		Reflect::TypeHandle assetTypeHandle;
	};
}