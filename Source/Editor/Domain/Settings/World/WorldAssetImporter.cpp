#include "WorldAssetImporter.h"

#include <Editor/Domain/Importer/AssetImportContext.h>
#include <Editor/Domain/Importer/ImporterDefinitions.h>

namespace Horizon
{
	void WorldAssetImporter::OnImport(AssetImportContext& context)
	{
	}

	void WorldAssetImporter::OnImportDefault(AssetImportContext& context)
	{
	}
}

HORIZON_SCRIPTED_IMPORTER(WorldAssetImporter, "DefaultScene", ".hworld", 1);