#pragma once

#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Editor/ImportPipeline/Mesh/GltfDefinitions.h>
#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(AssetImporterType[Reflect::TypeOf<Engine::MeshAsset>(), { ".gltf", ".glb" }]);
	class EDITOR_API GltfMeshAssetImporter final : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(GltfMeshAssetImporter);
	public:
		GltfMeshAssetImporter() = default;
		~GltfMeshAssetImporter() = default;

		AssetImportResult Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes) final;
		void Release() final;

		usize GetProductCount() const final;
		ImportDeclaredProduct GetProduct(usize index) const final;
		void GatherDependencies(const ImportPipelineContext& ctx, List<ImportDependency>& outDependencies) final;

		AssetImportResult GenerateProperties(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) final;
		AssetImportResult GenerateTable(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) final;
		AssetImportResult GenerateBinary(const ImportPipelineContext& ctx, usize product, List<u8>& outBytes) final;

	private:
		gltfParserPtr m_parser;
		gltfAssetPtr m_asset;
	};
}