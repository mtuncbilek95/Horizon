#pragma once

#include <Engine/Asset/Mesh/MeshProperties.h>
#include <Runtime/Containers/Vector/ReadOnlyList.h>

namespace Horizon::Asset
{
	class Mesh;

	struct MeshUsePermit
	{
		Mesh* pAsset = nullptr;
		ReadOnlyList<MeshSubmesh> submeshes;
		u32 firstVertex = 0;
		u32 firstIndex = 0;

		b8 IsValid() const { return pAsset != nullptr; }
	};
}