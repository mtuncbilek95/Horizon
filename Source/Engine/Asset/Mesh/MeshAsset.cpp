#include "MeshAsset.h"

#include <Engine/Asset/Mesh/MeshAssetStreamer.h>

namespace Horizon::Engine
{
	void MeshAsset::LoadAsync()
	{
		if (m_residency.CompareExchange(AssetResidency::Unloaded, AssetResidency::Pending) != AssetResidency::Unloaded)
			return;

		m_streamer->LoadAsync(this);
	}

	void MeshAsset::UnloadAsync()
	{
	}
}