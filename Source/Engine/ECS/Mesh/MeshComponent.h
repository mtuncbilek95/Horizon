#pragma once

#include <memory>

namespace Horizon
{
	class GfxMesh;
	class GfxMaterial;

	class MeshComponent
	{
	private:
		std::shared_ptr<GfxMesh> mesh;
		std::shared_ptr<GfxMaterial> material;
	};
}
