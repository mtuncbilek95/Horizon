#include "MeshAsset.h"

#include <Engine/Asset/Mesh/MeshAssetStreamer.h>

namespace Horizon::Engine
{
	void MeshAsset::LoadAsync()
	{
		if (m_residency.CompareExchange(AssetResidency::Unloaded, AssetResidency::Pending) != AssetResidency::Unloaded)
			return;

		GetLoader<MeshAssetStreamer>()->LoadAsync(this);
	}

	void MeshAsset::UnloadAsync()
	{
		if (m_residency.CompareExchange(AssetResidency::Resident, AssetResidency::Evicting) != AssetResidency::Resident)
			return;

		GetLoader<MeshAssetStreamer>()->UnloadAsync(this);
	}

	void MeshAsset::OnPropertiesChanged(ReadOnlyList<u8> propertyBytes)
	{
	}
}