#pragma once

#include <Engine/Asset/AssetKind.h>
#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetResidency.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/Material/MaterialUsePermit.h>
#include <Engine/Asset/Material/MaterialProperties.h>
#include <Engine/Job/JobSystem.h>

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/Memory/RangeAllocator.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/PAL/Sync/Atomic.h>

#include <string>

namespace Horizon::Asset
{
	HCLASS(Engine::AssetType[Engine::AssetKind::Streamable]);
	class Material : public Engine::AssetObject
	{
		HORIZON_TYPE_REFLECT(Material);
		friend class MaterialStreamer;
	public:
		Material() = default;
		~Material() = default;

		void LoadAsync();
		void UnloadAsync();

		MaterialUsePermit BeginUse();
		void EndUse(MaterialUsePermit& permit);

		const Guid& GetId() const { return m_id; }
		Engine::AssetResidency GetResidency() const { return m_residency.Load(PAL::MemoryOrder::Acquire); }

	protected:
		void OnPropertiesChanged(ReadOnlyList<u8> propertyBytes);

	private:
		void ReleaseUse();

	private:
		Guid m_id;
		std::string m_cookPath;
		PAL::Atomic<Engine::AssetResidency> m_residency = Engine::AssetResidency::Unloaded;
		Engine::SubmitTicket m_ticket = Engine::InvalidSubmitTicket;

		MaterialProperties m_properties;
	};
}