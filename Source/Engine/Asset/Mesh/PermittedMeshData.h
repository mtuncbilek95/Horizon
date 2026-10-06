#pragma once

#include <Engine/Asset/Mesh/MeshSubMesh.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::Engine
{
	class MeshAsset;

	struct PermittedMeshData
	{
		MeshAsset* pAsset = nullptr;
		const MeshSubMesh* pSubMeshes = nullptr;
		u32 subMeshCount = 0;
		u32 firstVertex = 0;
		u32 firstIndex = 0;

		b8 IsValid() const { return pAsset != nullptr; }
	};
}