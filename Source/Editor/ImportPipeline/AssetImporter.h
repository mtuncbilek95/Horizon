#pragma once

#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/ImportPipelineContext.h>
#include <Editor/ImportPipeline/ImportDeclaredProduct.h>
#include <Editor/ImportPipeline/ImportDependency.h>
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

		virtual usize GetProductCount() const = 0;
		virtual ImportDeclaredProduct GetProduct(usize index) const = 0;
		virtual void GatherDependencies(const ImportPipelineContext& ctx, List<ImportDependency>& outDependencies) {}

		virtual AssetImportResult GenerateProperties(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) = 0;
		virtual AssetImportResult GenerateTable(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) = 0;
		virtual AssetImportResult GenerateBinary(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) = 0;
	};
}