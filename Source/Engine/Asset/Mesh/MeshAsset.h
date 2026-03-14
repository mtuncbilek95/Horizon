#pragma once

#include <Engine/Asset/Asset.h>
#include <Engine/Asset/Mesh/Geometry.h>

namespace Horizon
{
	class MeshImporter;

	class MeshAsset : public Asset<MeshAsset>
	{
	public:
		MeshAsset() = default;
		virtual ~MeshAsset() = default;

		bool Load() final;
		void Unload() final;
		bool IsLoaded() const final { return m_loaded; }

		const Geometry& GetGeometry() const { return m_geometry; }

	private:
		Geometry m_geometry;
		bool m_loaded = false;

		friend class MeshImporter;
	};
}
