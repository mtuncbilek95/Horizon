#include "ShaderAsset.h"

#include <Engine/Asset/Shader/ShaderAssetLoader.h>

namespace Horizon::Engine
{
	void ShaderAsset::LoadSync()
	{
		if (m_residency.CompareExchange(AssetResidency::Unloaded, AssetResidency::Pending) != AssetResidency::Unloaded)
			return;

		GetLoader<ShaderAssetLoader>()->LoadSync(this);
	}

	void ShaderAsset::UnloadSync()
	{
		if (m_residency.CompareExchange(AssetResidency::Resident, AssetResidency::Evicting) != AssetResidency::Resident)
			return;

		GetLoader<ShaderAssetLoader>()->UnloadSync(this);
	}
}