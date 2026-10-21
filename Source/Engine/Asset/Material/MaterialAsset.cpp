#include "MaterialAsset.h"

#include <Engine/Asset/Material/MaterialAssetLoader.h>

namespace Horizon::Engine
{
	void MaterialAsset::LoadSync()
	{
		if (m_residency.CompareExchange(AssetResidency::Unloaded, AssetResidency::Pending) != AssetResidency::Unloaded)
			return;

		GetLoader<MaterialAssetLoader>()->LoadSync(this);
	}

	void MaterialAsset::UnloadSync()
	{
		if (m_residency.CompareExchange(AssetResidency::Resident, AssetResidency::Evicting) != AssetResidency::Resident)
			return;

		GetLoader<MaterialAssetLoader>()->UnloadSync(this);
	}
}