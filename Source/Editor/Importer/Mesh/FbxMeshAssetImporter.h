#pragma once

#include <Editor/Attributes/ImportTypeAttribute.h>
#include <Editor/Importer/AssetImporter.h>
#include <Engine/Asset/Mesh/MeshProperties.h>

namespace Horizon::Editor
{
	HCLASS(ImportTypeAttribute[Reflect::TypeOf<Engine::MeshAsset>(), { ".fbx" }]);
	class H_EXPORT FbxMeshAssetImporter : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(FbxMeshAssetImporter);
	public:
		AssetImportResult ImportAsset(Engine::Engine* pEngine, const std::string& inPath, List<u8>& outByteArr) final;
		usize GetPropertySize() const final { return sizeof(Engine::MeshProperties); }
	};
}