#pragma once

#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Editor/ImportPipeline/Mesh/FbxDefinitions.h>
#include <Engine/Asset/Mesh/Mesh.h>
#include <Engine/Asset/Mesh/MeshProperties.h>
#include <Engine/Asset/Mesh/MeshVertex.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(AssetImporterType[Reflect::TypeOf<Asset::Mesh>(), { ".fbx" }]);
	class FbxMeshAssetImporter final : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(FbxMeshAssetImporter);
	public:
		FbxMeshAssetImporter() = default;
		~FbxMeshAssetImporter() = default;

		AssetImportResult Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes) final;
		void Release() final;

		AssetImportResult GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes) final;
		AssetImportResult GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes) final;
		AssetImportResult GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes) final;

		void CollectDependencies(const ImportPipelineContext& ctx, List<std::string>& outPaths) final;
		void CollectMaterials(const ImportPipelineContext& ctx, List<ImportedMaterial>& outMaterials) final;

	private:
		AssetImportResult BuildMesh(const ImportPipelineContext& ctx);
		void GenerateSmoothNormals(const Asset::MeshSubmesh& subMesh);
		void GenerateTangents(const Asset::MeshSubmesh& subMesh, b8 hasSourceTangents);

	private:
		ufbxScenePtr m_sceneNode;

		List<Asset::MeshVertex> m_vertices;
		List<u32> m_indices;
		List<Asset::MeshSubmesh> m_subMeshes;
		b8 m_hasBuilt = false;
	};
}