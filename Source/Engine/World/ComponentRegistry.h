#pragma once

#include <Engine/World/ComponentStorage.h>
#include <Engine/World/Definitions.h>

#include <Runtime/Containers/List.h>

#include <unordered_map>

namespace Horizon::Engine
{
	class ReflectionSystem;

	class H_EXPORT ComponentRegistry final
	{
	public:
		ComponentRegistry(ReflectionSystem* pReflection) : m_reflection(pReflection)
		{
		}
		~ComponentRegistry();

		ComponentRegistry(const ComponentRegistry&) = delete;
		ComponentRegistry& operator=(const ComponentRegistry&) = delete;

		ComponentStorage* GetOrCreateStorage(ComponentTypeId typeId);
		ComponentStorage* GetOrCreateStorage(Reflect::Type& type);
		ComponentStorage* FindStorage(ComponentTypeId typeId) const;
		u32 FindSlot(ComponentTypeId typeId) const;

		const List<ComponentStorage*>& GetStorages() const { return m_storages; }

	private:
		ReflectionSystem* m_reflection = nullptr;
		std::unordered_map<ComponentTypeId, u32> m_slots;
		List<ComponentStorage*> m_storages;
	};
}