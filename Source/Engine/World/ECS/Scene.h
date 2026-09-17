#pragma once

#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/World/ECS/EntityStorage.h>
#include <Engine/World/ECS/ComponentRegistry.h>
#include <Engine/World/ECS/ComponentObject.h>

#include <Runtime/Containers/List.h>
#include <Runtime/Containers/StringOps.h>

namespace Horizon::Engine
{
	class H_EXPORT Scene
	{
	public:
		Scene(ReflectionSystem* pReflection);
		~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		EntityHandle AddEntity();
		void RemoveEntity(EntityHandle handl);
		void RemoveEntity(usize index);
		b8 IsAlive(EntityHandle handl) const;

		ComponentObject* AddComponent(EntityHandle handl, ComponentTypeId typeId);
		void RemoveComponent(EntityHandle handl, ComponentTypeId typeId);
		ComponentObject* FindComponent(EntityHandle handl, ComponentTypeId typeId) const;
		b8 HasComponent(EntityHandle handl, ComponentTypeId typeId) const;

		template<typename T>
		T* AddComponent(EntityHandle handl, T&& comp)
		{
			ComponentStorage* pStorage = ResolveStorage(Reflect::TypeOf<T>());
			if (!pStorage)
				return nullptr;

			return static_cast<T*>(AddComponentTo(pStorage, handl, &comp));
		}

		template<typename T>
		void RemoveComponent(EntityHandle handl)
		{
			RemoveComponent(handl, Reflect::TypeOf<T>());
		}

		template<typename T>
		T* FindComponent(EntityHandle handl) const
		{
			return static_cast<T*>(FindComponent(handl, Reflect::TypeOf<T>()));
		}

		template<typename T>
		b8 HasComponent(EntityHandle handl) const
		{
			return HasComponent(handl, Reflect::TypeOf<T>());
		}

		template<typename... Ts, typename Fn>
		void ForEach(Fn&& func)
		{
			if constexpr (sizeof...(Ts) == 1)
			{
				ComponentStorage* pStorage = m_components.FindStorage(Reflect::TypeOf<Ts...>());

				if (!pStorage)
					return;

				for (usize i = 0; i < pStorage->GetCount(); i++)
					func(pStorage->GetEntityAt(i), *static_cast<Ts*>(pStorage->GetAt(i))...);
			}
			else
			{
				ComponentStorage* storages[] = { m_components.FindStorage(Reflect::TypeOf<Ts>())... };

				for (ComponentStorage* pStorage : storages)
				{
					if (!pStorage)
						return;
				}

				ComponentStorage* pDriver = storages[0];

				for (ComponentStorage* pStorage : storages)
				{
					if (pStorage->GetCount() < pDriver->GetCount())
						pDriver = pStorage;
				}

				for (usize i = 0; i < pDriver->GetCount(); i++)
				{
					const EntityHandle entity = pDriver->GetEntityAt(i);

					[&](auto*... pComps)
						{
							if ((pComps && ...))
								func(entity, *pComps...);
						}(FindComponent<Ts>(entity)...);
				}
			}
		}

		EntityStorage& GetEntities() { return m_entities; }
		const EntityStorage& GetEntities() const { return m_entities; }
		ComponentRegistry& GetComponents() { return m_components; }
		const ComponentRegistry& GetComponents() const { return m_components; }

		u32 GetEntityCount() const { return m_entities.GetAliveCount(); }

	private:
		ComponentStorage* ResolveStorage(ComponentTypeId typeId);
		ComponentObject* AddComponentTo(ComponentStorage* pStorage, EntityHandle handl, void* pSource);

	private:
		ReflectionSystem* m_reflection = nullptr;
		EntityStorage m_entities;
		ComponentRegistry m_components;
	};
}