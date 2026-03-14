#include "MeshAsset.h"
#include "MeshImporter.h"

namespace Horizon
{
	bool MeshAsset::Load()
	{
		if (m_loaded)
			return true;

		MeshImporter importer;
		if (!importer.Import(*this, m_assetPath))
			return false;

		m_loaded = true;
		return true;
	}

	void MeshAsset::Unload()
	{
		m_geometry = Geometry{};
		m_loaded   = false;
	}
}
