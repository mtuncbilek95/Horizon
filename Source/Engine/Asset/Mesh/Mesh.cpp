#include "Mesh.h"

#include <Engine/Asset/Mesh/MeshStreamer.h>

namespace Horizon::Asset
{
	void Mesh::LoadAsync()
	{
		if (m_residency.CompareExchange(Engine::AssetResidency::Unloaded, Engine::AssetResidency::Pending) != Engine::AssetResidency::Unloaded)
			return;

		GetLoader<MeshStreamer>()->LoadAsync(this);
	}

	void Mesh::UnloadAsync()
	{
		if (m_residency.CompareExchange(Engine::AssetResidency::Resident, Engine::AssetResidency::Evicting) != Engine::AssetResidency::Resident)
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
			GetLoader<MeshStreamer>()->UnloadAsync(this);
	}

	MeshUsePermit Mesh::BeginUse()
	{
		if (GetResidency() != Engine::AssetResidency::Resident)
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

		if (GetResidency() != Engine::AssetResidency::Resident)
		{
			ReleaseUse();
			return {};
		}

		return { this, m_submeshes, GetFirstVertex(), GetFirstIndex() };
	}

	void Mesh::EndUse(MeshUsePermit& permit)
	{
		if (permit.pAsset != this)
			return;

		permit = {};
		ReleaseUse();
	}

	void Mesh::ReleaseUse()
	{
		const u32 prev = m_useCount.FetchSubtract(1, PAL::MemoryOrder::AcqRel);
		if (prev == ((1u << 31u) | 1))
			GetLoader<MeshStreamer>()->UnloadAsync(this);
	}

	void Mesh::OnPropertiesChanged(ReadOnlyList<u8> propertyBytes)
	{
	}
}