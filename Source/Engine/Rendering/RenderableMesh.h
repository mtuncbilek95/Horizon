#pragma once

#include <Engine/Asset/Mesh/MeshUsePermit.h>
#include <Runtime/Math/Mat4f.h>
#include <Runtime/Math/Vec3f.h>

namespace Horizon::Engine 
{
	struct RenderableMesh
	{
		Math::Mat4f worldMatrix;
		Asset::MeshUsePermit mesh;

		b8 visible;
		b8 castShadow;
	};
}