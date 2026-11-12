#pragma once

#include <Engine/Asset/Mesh/MeshUsePermit.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	struct RenderableMesh
	{
		Math::Mat4f worldMatrix;
		Asset::MeshUsePermit mesh;

		u32 firstMaterial;
		u32 materialCount;

		b8 visible;
		b8 castShadow;
	};

	struct FrameMesh
	{
		List<RenderableMesh> meshes;
		List<u32> materialIndices;

		u32 materialBufferView = kInvalid32;
		u32 samplerView = kInvalid32;
	};
}