#pragma once

#include <Engine/Asset/Importer/Importer.h>
#include <Engine/Asset/Material/MaterialAsset.h>

namespace Horizon
{
	class MaterialImporter : public Importer<MaterialAsset>
	{
	public:
		bool Import(MaterialAsset& asset, const std::filesystem::path& path) override;
	};
}
