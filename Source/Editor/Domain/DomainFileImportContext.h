#pragma once

#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/ImportPipeline.h>
#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Editor/ImportPipeline/ImportReason.h>
#include <Engine/Job/JobTicket.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Editor
{
	enum class ImportPhase : u8
	{
		Pending,
		Parsing,
		Cooking
	};

	struct DomainFileImportContext
	{
		std::string relativePath;

		AssetImporter* pImporter = nullptr;
		AssetImportSettings* pSettings = nullptr;
		Reflect::Type* pSettingsType = nullptr;

		ImportPipeline* pPipeline = nullptr;
		Reflect::Type* pAssetType = nullptr;
		std::string cookedPath;

		ImportPipelineContext context;

		ImportPhase phase = ImportPhase::Pending;
		Engine::SubmitTicket ticket = Engine::InvalidSubmitTicket;
		AssetImportResult result = AssetImportResult::InternalError;
		b8 requeue = false;
	};
}