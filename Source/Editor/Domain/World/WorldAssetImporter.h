#pragma once

#include <Editor/Domain/Importer/IAssetImporter.h>
#include <Editor/Domain/Importer/AssetImporterAttribute.h>

#include <vector>
#include <string_view>

#include "WorldAssetImporter.reflected.h"

namespace Horizon
{
	HCLASS(AssetImporterAttribute[std::vector<std::string_view>{ ".hworld" }, "DefaultScene", 1u])
	class WorldAssetImporter : public IAssetImporter
	{
		HORIZON_REFLECT;
	public:
		void OnImport(AssetImportContext& context) final;
		void OnImportDefault(AssetImportContext& context) final;
	};
}