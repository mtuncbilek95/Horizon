#pragma once

#include <Editor/ImportPipeline/ImportPipelineInfoAttribute.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Containers/ReadOnlyList.h>
#include <Runtime/RTTR/Reflection.h>

#include <string>

namespace Horizon::Editor
{
	class EDITOR_API AssetImporter : Reflect::Base
	{
	public:
		virtual AssetImportResult CookAsset(Engine::Engine* pEngine, ReadOnlyList<u8> fileBytes, AssetImportSettings* pSettings, List<u8>& outBytes, usize& outPropertySize) = 0;
		virtual b8 PatchCook(const AssetImportSettings* pPrevious, const AssetImportSettings* pCurrent, const std::string cookedPath) = 0;
	};
}