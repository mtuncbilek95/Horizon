#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/RTTR/Reflection.h>

#include <filesystem>
#include <string>
#include <vector>

namespace Horizon
{
	HCLASS();
	struct H_EXPORT MetaHeader
	{
		HFIELD();
		Guid id;
		HFIELD();
		std::string assetTypeName;
		HFIELD();
		std::filesystem::path cookedPath;
	};
}