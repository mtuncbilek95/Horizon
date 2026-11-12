#pragma once

#include <Engine/Asset/AssetKind.h>
#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetResidency.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/Material/MaterialUsePermit.h>
#include <Engine/Asset/Material/MaterialParameters.h>
#include <Engine/Asset/Material/MaterialProperties.h>
#include <Engine/Job/JobSystem.h>

#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/PAL/Sync/Atomic.h>

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

		const MaterialProperties& GetProperties() const { return m_properties; }
		const MaterialParameters& GetParameters() const { return m_parameters; }

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
		MaterialParameters m_parameters;

		u32 m_slotIndex = kInvalid32;
		PAL::Atomic<u32> m_useCount = 0;
	};
}