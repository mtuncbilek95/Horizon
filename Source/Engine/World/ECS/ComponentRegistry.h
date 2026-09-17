#pragma once

#include <Engine/World/ECS/ComponentStorage.h>
#include <Engine/World/ECS/Definitions.h>

#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

#include <unordered_map>

namespace Horizon::Engine
{
	class H_EXPORT ComponentRegistry final
	{
	public:
		ComponentRegistry() = default;
		~ComponentRegistry();

		ComponentRegistry(const ComponentRegistry&) = delete;
		ComponentRegistry& operator=(const ComponentRegistry&) = delete;

		ComponentStorage* GetOrCreateStorage(const Reflect::Type* pType);
		ComponentStorage* FindStorage(ComponentTypeId typeId) const;
		u32 FindSlot(ComponentTypeId typeId) const;

		const List<ComponentStorage*>& GetStorages() const { return m_storages; }

	private:
		std::unordered_map<ComponentTypeId, u32> m_slots;
		List<ComponentStorage*> m_storages;
	};
}