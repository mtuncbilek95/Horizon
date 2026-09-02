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
		Scene();
		~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;

		EntityHandle AddEntity();
		void RemoveEntity(EntityHandle handl);
		b8 IsAlive(EntityHandle handl) const;

		template<typename T>
		T* AddComponent(EntityHandle handl, T&& comp)
		{
			if (!m_entities.IsAlive(handl))
			{
				Terminal::Warn(StringOps::GetName(this), "AddComponent on a dead or stale entity");
				return nullptr;
			}

			ComponentStorage<T>* pStorage = m_components.GetOrCreateStorage<T>();
			if (!pStorage)
				return nullptr;

			T& stored = pStorage->Insert(handl, std::move(comp));
			m_entities.GetSignatureOf(handl)->set(pStorage->GetSlot());

			return &stored;
		}

		template<typename T>
		void RemoveComponent(EntityHandle handl)
		{
			if (!m_entities.IsAlive(handl))
				return;

			ComponentStorage<T>* pStorage = m_components.FindStorage<T>();

			if (!pStorage)
				return;

			pStorage->Remove(handl);

			if (Signature* pSignature = m_entities.GetSignatureOf(handl))
				pSignature->reset(pStorage->GetSlot());
		}

		template<typename T>
		T* FindComponent(EntityHandle handl)
		{
			if (!m_entities.IsAlive(handl))
				return nullptr;

			ComponentStorage<T>* pStorage = m_components.FindStorage<T>();
			if (!pStorage)
				return nullptr;

			return pStorage->Find(handl);
		}

		template<typename T>
		b8 HasComponent(EntityHandle handl) const
		{
			if (!m_entities.IsAlive(handl))
				return false;

			u32 slot = m_components.FindSlot(Reflect::TypeOf<T>());
			if (slot == kInvalid32)
				return false;

			return m_entities.GetSignatureOf(handl)->test(slot);
		}

		template<typename... Ts, typename Fn>
		void ForEach(Fn&& func)
		{
			if constexpr (sizeof...(Ts) == 1)
			{
				auto* pStorage = m_components.FindStorage<Ts...>();

				if (!pStorage)
					return;

				for (usize i = 0; i < pStorage->GetCount(); i++)
					func(pStorage->GetEntityAt(i), pStorage->GetAt(i));
			}
			else
			{
				IComponentStorage* storages[] = { m_components.FindStorage<Ts>()... };

				for (IComponentStorage* pStorage : storages)
				{
					if (!pStorage)
						return;
				}

				IComponentStorage* pDriver = storages[0];

				for (IComponentStorage* pStorage : storages)
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
		ComponentRegistry& GetComponents() { return m_components; }

		u32 GetEntityCount() const { return m_entities.GetAliveCount(); }

	private:
		EntityStorage m_entities;
		ComponentRegistry m_components;
	};
}