#pragma once

#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Editor/ImportPipeline/Mesh/GltfDefinitions.h>
#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Engine/Asset/Mesh/MeshAssetProperties.h>
#include <Engine/Asset/Mesh/MeshVertex.h>
#include <Runtime/Containers/List.h>
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

		AssetImportResult GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes) final;
		AssetImportResult GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes) final;
		AssetImportResult GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes) final;

	private:
		b8 LoadExternalBuffers(const ImportPipelineContext& ctx);
		AssetImportResult BuildMesh(const ImportPipelineContext& ctx);

	private:
		gltfParserPtr m_parser;
		gltfAssetPtr m_asset;

		List<Engine::MeshVertex> m_vertices;
		List<u32> m_indices;
		List<Engine::MeshAssetSubmesh> m_subMeshes;
		b8 m_hasBuilt = false;
	};
}