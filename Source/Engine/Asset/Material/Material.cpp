#include "Material.h"

#include <Engine/Asset/Material/MaterialStreamer.h>

namespace Horizon::Asset
{
	void Material::LoadAsync()
	{
		if (m_residency.CompareExchange(Engine::AssetResidency::Unloaded, Engine::AssetResidency::Pending) != Engine::AssetResidency::Unloaded)
			return;

		GetLoader<MaterialStreamer>()->LoadAsync(this);
	}

	void Material::UnloadAsync()
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
			GetLoader<MaterialStreamer>()->UnloadAsync(this);
	}

	MaterialUsePermit Material::BeginUse()
	{
		if (GetResidency() != Engine::AssetResidency::Resident)
			return {};

		u32 current = m_useCount.Load(PAL::MemoryOrder::Acquire);
		while (true)
		{
			if (current & (1u << 31u))
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

		if (current == 0)
			GetLoader<MaterialStreamer>()->AcquireTextures(this);

		return { this, m_slotIndex };
	}

	void Material::EndUse(MaterialUsePermit& permit)
	{
		if (permit.pAsset != this)
			return;

		permit = {};
		ReleaseUse();
	}

	void Material::OnPropertiesChanged(ReadOnlyList<u8> propertyBytes)
	{
	}

	void Material::ReleaseUse()
	{
		const u32 prev = m_useCount.FetchSubtract(1, PAL::MemoryOrder::AcqRel);

		if ((prev & ~(1u << 31u)) == 1)
			GetLoader<MaterialStreamer>()->ReleaseTextures(this);

		if (prev == ((1u << 31u) | 1))
			GetLoader<MaterialStreamer>()->UnloadAsync(this);
	}
}