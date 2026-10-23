#pragma once

#include <Engine/Asset/AssetKind.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetResidency.h>
#include <Engine/Job/JobSystem.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/PAL/Sync/Atomic.h>

#include <string>

namespace Horizon::Engine
{
	HCLASS(AssetType[AssetKind::Streamable]);
	class ENGINE_API TextureAsset : public AssetObject
	{
		HORIZON_TYPE_REFLECT(TextureAsset);
		friend class TextureAssetStreamer;
	public:
		TextureAsset();
		~TextureAsset();

		void LoadAsync();
		void UnloadAsync();

		const Guid& GetId() const { return m_id; }
		AssetResidency GetResidency() const { return m_residency.Load(PAL::MemoryOrder::Acquire); }

	protected:
		void OnPropertiesChanged(ReadOnlyList<u8> propertyBytes) {}

	private:
		Guid m_id;
		std::string m_cookPath;
		PAL::Atomic<AssetResidency> m_residency = AssetResidency::Unloaded;
		SubmitTicket m_ticket = InvalidSubmitTicket;
		
	};
}