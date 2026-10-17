#pragma once

#include <Editor/ImportPipeline/ImportDependencyKind.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/RTTR/Reflection.h>
#include <string>

namespace Horizon::Editor
{
	struct ImportDependency
	{
		ImportDependencyKind kind = ImportDependencyKind::Data;
		std::string sourceRelativePath;
		std::string subAssetKey;
		std::string targetRelativePath;
		Reflect::TypeHandle assetType = {};
		Guid resolvedId;
	};
}