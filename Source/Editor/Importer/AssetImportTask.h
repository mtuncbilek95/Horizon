#pragma once

#include <Editor/Importer/AssetImportResult.h>
#include <Engine/Job/JobTicket.h>
#include <Runtime/Containers/Guid.h>

#include <string>

namespace Horizon::Engine
{
	class Engine;
}

namespace Horizon::Editor
{
	class AssetImporter;

	struct ImportTask
	{
		Guid id;
		std::string relativePath;
		std::string sourcePath;
		std::string cookedPath;
		std::string assetTypeName;

		AssetImporter* pImporter = nullptr;
		Engine::Engine* pEngine = nullptr;

		Engine::SubmitTicket ticket = Engine::InvalidSubmitTicket;

		AssetImportResult result = AssetImportResult::InternalError;
		b8 wasCooked = false;
	};
}
