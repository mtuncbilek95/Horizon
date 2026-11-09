#pragma once

#include <Engine/Asset/AssetKind.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetResidency.h>
#include <Engine/Job/JobSystem.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/PAL/Sync/Atomic.h>

#include <string>

namespace Horizon::Asset
{
	HCLASS(Engine::AssetType[Engine::AssetKind::Streamable]);
	class Texture : public Engine::AssetObject
	{
		HORIZON_TYPE_REFLECT(Texture);
		friend class TextureStreamer;
	public:
		Texture();
		~Texture();

		void LoadAsync();
		void UnloadAsync();

		const Guid& GetId() const { return m_id; }
		Engine::AssetResidency GetResidency() const { return m_residency.Load(PAL::MemoryOrder::Acquire); }

	protected:
		void OnPropertiesChanged(ReadOnlyList<u8> propertyBytes) {}

	private:
		Guid m_id;
		std::string m_cookPath;
		PAL::Atomic<Engine::AssetResidency> m_residency = Engine::AssetResidency::Unloaded;
		Engine::SubmitTicket m_ticket = Engine::InvalidSubmitTicket;
	};
}