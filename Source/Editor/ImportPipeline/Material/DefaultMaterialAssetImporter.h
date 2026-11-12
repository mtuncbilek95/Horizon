#pragma once

#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Engine/Asset/Material/Material.h>
#include <Engine/Asset/Material/MaterialParameters.h>
#include <Engine/Asset/Material/MaterialProperties.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(AssetImporterType[Reflect::TypeOf<Asset::Material>(), { ".hmaterial" }]);
	class DefaultMaterialAssetImporter final : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(DefaultMaterialAssetImporter);
	public:
		DefaultMaterialAssetImporter() = default;
		~DefaultMaterialAssetImporter();

		AssetImportResult Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes) final;
		void Release() final;

		AssetImportResult GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes) final;
		AssetImportResult GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes) final;
		AssetImportResult GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes) final;

	private:
		Asset::MaterialProperties m_properties;
		Asset::MaterialParameters m_parameters;
		b8 m_parsed = false;
	};
}