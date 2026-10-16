#pragma once

#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/ImportPipeline.h>
#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
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
		SettingsChanged
	};

	struct DomainImportTask
	{
		~DomainImportTask()
		{
			Memory::Allocator::Delete(pImporter);
			Memory::Allocator::Delete(pSettings);
		}

		Guid id;
		std::string relativePath;
		std::string sourcePath;
		std::string cookedPath;

		ImportPipeline* pPipeline = nullptr;
		AssetImporter* pImporter = nullptr;
		Reflect::Type* pAssetType = nullptr;
		Reflect::Type* pSettingsType = nullptr;
		AssetImportSettings* pSettings = nullptr;

		ImportReason reason = ImportReason::Fresh;
		Engine::SubmitTicket ticket = Engine::InvalidSubmitTicket;
		AssetImportResult result = AssetImportResult::InternalError;
		b8 requeue = false;
	};
}