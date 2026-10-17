#pragma once

#include <Editor/Domain/DomainCookedProduct.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/ImportPipeline.h>
#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Editor/ImportPipeline/ImportDependency.h>
#include <Editor/ImportPipeline/ImportDeclaredProduct.h>
#include <Engine/Job/JobTicket.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Editor
{
	enum class ImportReason : u8
	{
		Fresh,
		SourceChanged,
		SettingsChanged,
		DependencyChanged
	};

	enum class ImportPhase : u8
	{
		Pending,
		Parsing,
		Cooking
	};

	struct DomainFileImportContext
	{
		Guid id;
		std::string relativePath;
		std::string sourcePath;
		std::string cookedPath;

		ImportPipeline* pPipeline = nullptr;
		AssetImporter* pImporter = nullptr;
		AssetImportSettings* pSettings = nullptr;
		Reflect::Type* pAssetType = nullptr;
		Reflect::Type* pSettingsType = nullptr;

		List<ImportDeclaredProduct> expectedProducts;
		List<ImportDependency> dependencies;
		List<DomainCookedProduct> cookedProducts;

		ImportReason reason = ImportReason::Fresh;
		ImportPhase phase = ImportPhase::Pending;
		Engine::SubmitTicket ticket = Engine::InvalidSubmitTicket;
		AssetImportResult result = AssetImportResult::InternalError;
		b8 requeue = false;
	};
}