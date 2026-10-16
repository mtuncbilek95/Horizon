#pragma once

#include <Editor/ImportPipeline/AssetImporter.h>
#include <Editor/ImportPipeline/AssetImporterTypeAttribute.h>
#include <Editor/ImportPipeline/AssetImportResult.h>
#include <Editor/ImportPipeline/AssetImportSettings.h>
#include <Engine/Asset/Mesh/MeshAsset.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Editor
{
	HCLASS(AssetImporterType[Reflect::TypeOf<Engine::MeshAsset>()]);
	class EDITOR_API FbxMeshAssetImporter final : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(FbxMeshAssetImporter);
	public:
		FbxMeshAssetImporter() = default;
		~FbxMeshAssetImporter() = default;

		AssetImportResult CookAsset(Engine::Engine* pEngine, ReadOnlyList<u8> fileBytes, AssetImportSettings* pSettings, List<u8>& outBytes, usize& outPropertySize) final;
		b8 PatchCook(const AssetImportSettings* pPrevious, const AssetImportSettings* pCurrent, const std::string cookedPath) final;
	};
}