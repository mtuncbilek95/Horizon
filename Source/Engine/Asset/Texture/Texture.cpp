#include "Texture.h"

#include <Engine/Asset/Texture/TextureStreamer.h>

namespace Horizon::Asset
{
	Texture::Texture()
	{
	}

	Texture::~Texture()
	{
	}

	void Texture::LoadAsync()
	{
		if (m_residency.CompareExchange(Engine::AssetResidency::Unloaded, Engine::AssetResidency::Pending) != Engine::AssetResidency::Unloaded)
			return;

		GetLoader<TextureStreamer>()->LoadAsync(this);
	}

	void Texture::UnloadAsync()
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
			GetLoader<TextureStreamer>()->UnloadAsync(this);
	}

	TextureUsePermit Texture::BeginUse()
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

		return { this, m_context->GetView(), m_context->GetViewIndex() };
	}

	void Texture::EndUse(TextureUsePermit& permit)
	{
		if (permit.pAsset != this)
			return;

		permit = {};
		ReleaseUse();
	}

	void Texture::ReleaseUse()
	{
		const u32 prev = m_useCount.FetchSubtract(1, PAL::MemoryOrder::AcqRel);
		if (prev == ((1u << 31u) | 1))
			GetLoader<TextureStreamer>()->UnloadAsync(this);
	}
}