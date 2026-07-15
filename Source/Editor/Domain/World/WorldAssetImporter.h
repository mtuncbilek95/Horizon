#pragma once

#include <Editor/Domain/Importer/IAssetImporter.h>
#include <Editor/Domain/Importer/AssetImporterAttribute.h>

#include <Runtime/Reflection/Reflection.h>


namespace Horizon
{
	HCLASS(AssetImporterAttribute({ ".hworld" }, "DefaultWorld", 1));
	class WorldAssetImporter : public IAssetImporter
	{
	public:
		void OnImport(AssetImportContext& context) final;
		void OnImportDefault(AssetImportContext& context) final;
	};
}