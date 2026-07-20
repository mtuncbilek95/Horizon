#pragma once

#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <vector>

namespace Horizon
{
	HCLASS();
	struct H_EXPORT MetaAssetEntry
	{
		HFIELD();
		std::string sub;
		HFIELD();
		std::string type;
		HFIELD();
		std::string guid;
	};

	HCLASS();
	struct H_EXPORT MetaHeader
	{
		HFIELD();
		std::string source;
		HFIELD();
		std::vector<MetaAssetEntry> assets;
	};
}