#pragma once

#include <Engine/Asset/AssetKind.h>
#include <Engine/Asset/AssetTypeAttribute.h>
#include <Engine/Asset/AssetObject.h>
#include <Engine/Asset/AssetResidency.h>
#include <Engine/Asset/Input/InputAction.h>
#include <Engine/Job/JobSystem.h>
#include <Runtime/Containers/Guid.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/PAL/Sync/Atomic.h>

namespace Horizon::Asset
{
	HCLASS(Engine::AssetType[Engine::AssetKind::Data]);
	class InputActionMap : public Engine::AssetObject
	{
		HORIZON_TYPE_REFLECT(InputActionMap);
		friend class InputActionMapLoader;
	public:
		InputActionMap();
		~InputActionMap();

		void LoadSync();
		void UnloadSync();

		const Guid& GetId() const { return m_id; }
		Engine::AssetResidency GetResidency() const { return m_residency.Load(PAL::MemoryOrder::Acquire); }

		const List<InputAction>& GetActions() const { return m_actions; }
		i64 FindActionIndex(std::string_view name) const;

	protected:
		void OnPropertiesChanged(ReadOnlyList<u8> propertyBytes) {}

	private:
		Guid m_id;
		std::string m_cookPath;
		PAL::Atomic<Engine::AssetResidency> m_residency = Engine::AssetResidency::Unloaded;
		Engine::SubmitTicket m_ticket = Engine::InvalidSubmitTicket;
		List<InputAction> m_actions;
	};
}
