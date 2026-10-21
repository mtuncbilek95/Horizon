#pragma once

#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Engine/Asset/Shader/ShaderAsset.h>
#include <Engine/Asset/Shader/ShaderAssetProperties.h>
#include <Engine/Asset/Shader/ShaderAssetStage.h>
#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(AssetImporterType[Reflect::TypeOf<Engine::ShaderAsset>(), { ".hlsl" }]);
	class EDITOR_API DefaultShaderAssetImporter final : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(DefaultShaderAssetImporter);
	public:
		DefaultShaderAssetImporter() = default;
		~DefaultShaderAssetImporter() = default;

		AssetImportResult Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes) final;
		void Release() final;

		usize GetProductCount() const final;
		ImportDeclaredProduct GetProduct(usize index) const final;
		void GatherDependencies(const ImportPipelineContext& ctx, List<ImportDependency>& outDependencies) final;

		AssetImportResult GenerateProperties(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) final;
		AssetImportResult GenerateTable(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) final;
		AssetImportResult GenerateBinary(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) final;

	private:
		Engine::ShaderAssetProperties m_properties;
		List<Engine::ShaderAssetStage> m_stages;
		List<u8> m_payload;
	};
}