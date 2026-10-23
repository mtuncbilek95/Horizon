#pragma once

#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/ImportPipeline.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Editor
{
	struct DomainCookedProduct
	{
		std::string key;
		Guid id;
		Reflect::Type* pAssetType = nullptr;
		ImportPipeline* pPipeline = nullptr;
		std::string cookedPath;
		AssetImportResult result = AssetImportResult::InternalError;
	};
}