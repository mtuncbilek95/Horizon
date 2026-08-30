#pragma once

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <string>

namespace Horizon::Editor
{
	struct DomainSubAsset
	{
		std::string name;
		std::string assetTypeName;
		Guid id;
	};

	struct DomainMeta
	{
		static constexpr u32 Version = 1;

		Guid id;
		std::string assetTypeName;
		List<DomainSubAsset> subAssets;

		b8 Read(const std::string& metaPath);
		b8 Write(const std::string& metaPath) const;
	};
}