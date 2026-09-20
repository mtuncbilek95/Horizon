#pragma once

#include <Editor/Attributes/ImportTypeAttribute.h>
#include <Editor/Importer/AssetImporter.h>

namespace Horizon::Editor
{
	HCLASS(ImportTypeAttribute[Reflect::TypeOf<Engine::TextureAsset>(), { ".png" }]);
	class H_EXPORT PngTextureAssetImporter : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(PngTextureAssetImporter);
	public:
		AssetImportResult ImportAsset(Engine::Engine* pEngine, const std::string& inPath, List<u8>& outByteArr) final;
	};
}