#pragma once

#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Editor/ImportPipeline/Mesh/FbxDefinitions.h>
#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(AssetImporterType[Reflect::TypeOf<Engine::MeshAsset>(), { ".fbx" }]);
	class EDITOR_API FbxMeshAssetImporter final : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(FbxMeshAssetImporter);
	public:
		FbxMeshAssetImporter() = default;
		~FbxMeshAssetImporter() = default;

		AssetImportResult Parse(const ImportPipelineContext& ctx, ReadOnlyList<u8> fileBytes) final;
		void Release() final;

		AssetImportResult GenerateProperties(const ImportPipelineContext& ctx, List<u8>& outBytes);
		AssetImportResult GenerateTable(const ImportPipelineContext& ctx, List<u8>& outBytes);
		AssetImportResult GenerateBinary(const ImportPipelineContext& ctx, List<u8>& outBytes);

	private:
		ufbxScenePtr m_sceneNode;
	};
}