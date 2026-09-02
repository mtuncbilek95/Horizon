#pragma once

#include <Engine/World/ECS/ComponentStorage.h>
#include <Engine/World/ECS/Definitions.h>

#include <Runtime/Containers/List.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

#include <unordered_map>

namespace Horizon::Engine
{
	class H_EXPORT ComponentRegistry final
	{
	public:
		ComponentRegistry() = default;
		~ComponentRegistry();

		template<typename T>
		ComponentStorage<T>* GetOrCreateStorage()
		{
			ComponentTypeId typeId = Reflect::TypeOf<T>();

			auto it = m_slots.find(typeId);
			if(it != m_slots.end())
				return static_cast<ComponentStorage<T>*>(m_storages[it->second]);

			if (m_storages.GetCount() >= MaxComponents)
			{
				Terminal::Error(StringOps::GetName(this), "Component budget of {} is exhausted", MaxComponents);
				Terminal::Warn("Developer Info", "How the fuck did you need more than 256 component slots?? Re-assess your design god damn it.");
				return nullptr;
			}

			auto* pStorage = Memory::Allocator::Create<ComponentStorage<T>>(Memory::CurrLoc());
			if (!pStorage)
			{
				Terminal::Error(StringOps::GetName(this), "Storage could not be allocated");
				return nullptr;
			}

			u32 slot = (u32)m_storages.GetCount();
			pStorage->SetSlot(slot);
			m_slots[typeId] = slot;
			m_storages.PushBack(pStorage);

			return pStorage;
		}

		template<typename T>
		ComponentStorage<T>* FindStorage()
		{
			auto it = m_slots.find(Reflect::TypeOf<T>());
			if (it == m_slots.end())
				return nullptr;

			return static_cast<ComponentStorage<T>*>(m_storages[it->second]);
		}

		IComponentStorage* FindStorage(ComponentTypeId typeId);
		u32 FindSlot(ComponentTypeId typeId) const;

		const List<IComponentStorage*>& GetStorages() const { return m_storages; }

	private:
		std::unordered_map<ComponentTypeId, u32> m_slots;
		List<IComponentStorage*> m_storages;
	};
}