#pragma once

#include <Editor/Domain/Importer/IAssetImporter.h>

namespace Horizon
{
	class WorldAssetImporter : public IAssetImporter
	{
	public:
		void OnImport(AssetImportContext& context) final;
		void OnImportDefault(AssetImportContext& context) final;
	};
}