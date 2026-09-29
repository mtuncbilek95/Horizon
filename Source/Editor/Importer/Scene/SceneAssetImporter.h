#pragma once

#include <Editor/Importer/ImportTypeAttribute.h>
#include <Editor/Importer/AssetImporter.h>
#include <Engine/Asset/Scene/SceneProperties.h>

namespace Horizon::Editor
{
	HCLASS(ImportTypeAttribute[Reflect::TypeOf<Engine::SceneAsset>(), { ".pscene" }]);
	class H_EXPORT SceneAssetImporter : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(SceneAssetImporter);
	public:
		AssetImportResult ImportAsset(Engine::Engine* pEngine, const std::string& inPath, List<u8>& outByteArr) final;
		usize GetPropertySize() const final { return sizeof(Engine::SceneProperties); }
	};
}