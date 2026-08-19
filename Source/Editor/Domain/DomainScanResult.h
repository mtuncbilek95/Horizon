#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <string>

namespace Horizon::Editor
{
	struct DomainEntrySnapshot
	{
		std::string name;
		std::string absolutePath;
		std::string metaPath;
		std::string assetTypeName;
		Guid id;
		b8 isDirectory = false;
	};

	struct DomainFolderSnapshot
	{
		std::string relativePath;
		List<DomainEntrySnapshot> entries;
	};

	struct DomainScanResult
	{
		List<DomainFolderSnapshot> folders;
		b8 fullRescan = false;
	};
}