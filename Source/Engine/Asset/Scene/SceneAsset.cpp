#include "SceneAsset.h"

#include <Engine/Asset/Scene/SceneAssetStreamer.h>

namespace Horizon::Engine
{
	SceneAsset::SceneAsset()
	{
	}

	SceneAsset::~SceneAsset()
	{
		// TODO: Temporary
		if(m_scene)
			Memory::Allocator::Delete(m_scene);
	}

	void SceneAsset::LoadAsync()
	{
		if (m_residency.CompareExchange(AssetResidency::Unloaded, AssetResidency::Pending) != AssetResidency::Unloaded)
			return;

		m_streamer->LoadAsync(this);
	}

	void SceneAsset::UnloadAsync()
	{
		if (m_residency.CompareExchange(AssetResidency::Resident, AssetResidency::Pending) != AssetResidency::Resident)
			return;

		m_streamer->UnloadAsync(this);
	}
}