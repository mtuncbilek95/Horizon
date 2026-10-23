#pragma once

#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/ImportPipelineContext.h>
#include <Runtime/Containers/List.h>
#include <Runtime/Containers/ReadOnlyList.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	class AssetImporter : public Reflect::Base
	{
	public:
		virtual AssetImportResult Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes) = 0;
		virtual void Release() = 0;

		virtual AssetImportResult GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes) = 0;
		virtual AssetImportResult GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes) = 0;
		virtual AssetImportResult GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes) = 0;
	};
}