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

		u32 current = m_useCount.Load(PAL::MemoryOrder::Acquire);
		while (true)
		{
			const u32 prev = m_useCount.CompareExchange(current, current | (1u << 31u), PAL::MemoryOrder::AcqRel);
			if (prev == current)
				break;
			current = prev;
		}

		if (current == 0)
			GetLoader<MeshAssetStreamer>()->UnloadAsync(this);
	}

	MeshAssetUsePermit MeshAsset::BeginUse()
	{
		if (GetResidency() != AssetResidency::Resident)
			return {};

		u32 current = m_useCount.Load(PAL::MemoryOrder::Acquire);
		while (true)
		{
			if (current & (1u << 31u)) // Eviction flag
				return {};

			const u32 prev = m_useCount.CompareExchange(current, current + 1, PAL::MemoryOrder::AcqRel);
			if (prev == current)
				break;
			current = prev;
		}

		if (GetResidency() != AssetResidency::Resident)
		{
			ReleaseUse();
			return {};
		}

		return { this, m_submeshes, GetFirstVertex(), GetFirstIndex() };
	}

	void MeshAsset::EndUse(MeshAssetUsePermit& permit)
	{
		if (permit.pAsset != this)
			return;

		permit = {};
		ReleaseUse();
	}

	void MeshAsset::ReleaseUse()
	{
		const u32 prev = m_useCount.FetchSubtract(1, PAL::MemoryOrder::AcqRel);
		if (prev == ((1u << 31u) | 1))
			GetLoader<MeshAssetStreamer>()->UnloadAsync(this);
	}

	void MeshAsset::OnPropertiesChanged(ReadOnlyList<u8> propertyBytes)
	{
	}
}