#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Engine
{
	struct AssetPhysicalEntry
	{
		Guid assetId;
		std::string cookPath;

		Reflect::TypeHandle assetType;

	};
}