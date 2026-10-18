#pragma once

#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Editor/ImportPipeline/ImportDependency.h>
#include <Editor/ImportPipeline/ImportReason.h>
#include <Engine/Core/Engine.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>
#include <string_view>

namespace Horizon::Editor
{
	struct EDITOR_API ImportPipelineContext
	{
		std::string sourcePath;
		std::string sourceFolder;
		Guid fileId;
		ImportReason reason = ImportReason::Fresh;
		const AssetImportSettings* pSettings = nullptr;
		const List<ImportDependency>* pDependencies = nullptr; // Easy way out
		Engine::ReflectionSystem* pReflection = nullptr;

		Guid ResolveAssetRef(std::string_view sourceRelativePath, std::string_view subAssetKey = {}) const;
	};
}