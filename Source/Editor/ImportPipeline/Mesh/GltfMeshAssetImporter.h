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

		AssetImportResult CookAsset(Engine::Engine* pEngine, ReadOnlyList<u8> fileBytes, AssetImportSettings* pSettings, List<u8>& outBytes, usize& outPropertySize) final;
		b8 PatchCook(const AssetImportSettings* pPrevious, const AssetImportSettings* pCurrent, const std::string cookedPath) final;

	private:
		gltfParserPtr m_parser;
		gltfAssetPtr m_asset;
	};
}