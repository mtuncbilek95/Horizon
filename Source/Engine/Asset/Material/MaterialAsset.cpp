#include "MaterialAsset.h"
#include "MaterialImporter.h"

namespace Horizon
{
	bool MaterialAsset::Load()
	{
		if (m_loaded)
			return true;

		MaterialImporter importer;
		if (!importer.Import(*this, m_assetPath))
			return false;

		m_loaded = true;
		return true;
	}

	void MaterialAsset::Unload()
	{
		m_loaded = false;
	}
}
