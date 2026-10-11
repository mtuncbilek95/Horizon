#pragma once

#include <Editor/Importer/AssetImporter.h>
#include <Editor/Importer/ImportTypeAttribute.h>
#include <Engine/Asset/Script/ScriptAsset.h>
#include <Engine/Asset/Script/ScriptProperties.h>

namespace Horizon::Editor
{
	HCLASS(ImportTypeAttribute[Reflect::TypeOf<Engine::ScriptAsset>(), { ".hscript", ".lua" }]);
	class EDITOR_API ScriptAssetImporter : public AssetImporter
	{
		HORIZON_TYPE_REFLECT(ScriptAssetImporter);
	public:
		AssetImportResult ImportAsset(Engine::Engine* pEngine, const std::string& inPath, List<u8>& outByteArr) final;
		usize GetPropertySize() const final { return sizeof(Engine::ScriptProperties); }
	};
}