#include "TextureAsset.h"

#include <Engine/Asset/Texture/TextureAssetStreamer.h>

namespace Horizon::Engine
{
	void TextureAsset::LoadSync()
	{
		if (m_residency.CompareExchange(AssetResidency::Unloaded, AssetResidency::Pending) != AssetResidency::Unloaded)
			return;

		GetLoader<TextureAssetStreamer>()->LoadSync(this);
	}

	void TextureAsset::UnloadSync()
	{
		if (m_residency.CompareExchange(AssetResidency::Resident, AssetResidency::Evicting) != AssetResidency::Resident)
			return;

		GetLoader<TextureAssetStreamer>()->UnloadSync(this);
	}
}