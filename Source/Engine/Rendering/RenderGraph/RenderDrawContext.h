#pragma once

#include <Engine/Asset/Mesh/MeshAssetUsePermit.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine
{
	struct RenderableMesh
	{
		Math::Mat4f worldMatrix;
		MeshAssetUsePermit mesh;

		b8 visible;
		b8 castShadow;
	};

	struct RenderDrawContext final
	{
		Math::Mat4f camViewMatrix;
		Math::Mat4f camProjMatrix;
		Math::Mat4f camViewProjMatrix;
		Math::Vec3f camWorldPosition;

		List<RenderableMesh> meshes;
	};
}