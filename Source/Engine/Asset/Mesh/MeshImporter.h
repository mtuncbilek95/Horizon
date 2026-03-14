#pragma once

#include <Engine/Asset/Importer/Importer.h>
#include <Engine/Asset/Mesh/MeshAsset.h>

namespace Horizon
{
	class MeshImporter : public Importer<MeshAsset>
	{
	public:
		bool Import(MeshAsset& asset, const std::filesystem::path& path) override;
	};
}
