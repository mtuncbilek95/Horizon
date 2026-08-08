#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Containers/List.h>

#include <string>

namespace Horizon::PAL
{
	struct H_EXPORT Directory
	{
		struct Entry
		{
			std::string name;
			std::string fullPath;
			b8 isDirectory;
		};

		static b8 Create(const std::string& path);
		static b8 Delete(const std::string& path);
		static b8 Exists(const std::string& path);
		static List<Entry> Iterate(const std::string& path);
	};
}