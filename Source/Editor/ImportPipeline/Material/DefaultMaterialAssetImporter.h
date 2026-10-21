#pragma once

#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Editor/ImportPipeline/Material/MaterialDocument.h>
#include <Engine/Asset/Material/MaterialAsset.h>
#include <Engine/Asset/Material/MaterialAssetProperties.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(AssetImporterType[Reflect::TypeOf<Engine::MaterialAsset>(), { ".hmaterial" }]);
	class EDITOR_API DefaultMaterialAssetImporter final : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(DefaultMaterialAssetImporter);
	public:
		DefaultMaterialAssetImporter() = default;
		~DefaultMaterialAssetImporter() = default;

		AssetImportResult Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes) final;
		void Release() final;

		usize GetProductCount() const final;
		ImportDeclaredProduct GetProduct(usize index) const final;
		void GatherDependencies(const ImportPipelineContext& ctx, List<ImportDependency>& outDependencies) final;

		AssetImportResult GenerateProperties(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) final;
		AssetImportResult GenerateTable(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) final;
		AssetImportResult GenerateBinary(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) final;

	private:
		MaterialDocument m_document;
		Engine::MaterialAssetProperties m_properties;
		List<Engine::MaterialParamValue> m_values;
		b8 m_isParsed = false;
	};
}