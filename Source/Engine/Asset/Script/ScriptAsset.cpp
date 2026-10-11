#include "ScriptAsset.h"

#include <Engine/Asset/Script/ScriptAssetStreamer.h>

namespace Horizon::Engine
{
	void ScriptAsset::LoadAsync()
	{
		if (m_residency.CompareExchange(AssetResidency::Unloaded, AssetResidency::Pending) != AssetResidency::Unloaded)
			return;

		m_loadedRevision = m_revision.Load();
		m_streamer->LoadAsync(this);
	}

	void ScriptAsset::ReloadAsync()
	{
		if (m_residency.CompareExchange(AssetResidency::Failed, AssetResidency::Unloaded) != AssetResidency::Failed)
			UnloadAsync();

		LoadAsync();
	}

	void ScriptAsset::UnloadAsync()
	{
		if (m_residency.CompareExchange(AssetResidency::Resident, AssetResidency::Evicting) != AssetResidency::Resident)
			return;

		m_streamer->UnloadAsync(this);
	}
}