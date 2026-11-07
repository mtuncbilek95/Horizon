#pragma once

#include <Engine/Asset/Mesh/MeshAssetProperties.h>
#include <Runtime/Containers/Vector/ReadOnlyList.h>

namespace Horizon::Engine
{
	class MeshAsset;

	struct MeshAssetUsePermit
	{
		MeshAsset* pAsset = nullptr;
		ReadOnlyList<MeshAssetSubmesh> submeshes;
		u32 firstVertex = 0;
		u32 firstIndex = 0;

		b8 IsValid() const { return pAsset != nullptr; }
	};
}