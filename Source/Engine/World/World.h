#pragma once

#include <Engine/World/ComponentRegistry.h>
#include <Engine/World/Definitions.h>
#include <Engine/World/EntityStorage.h>

#include <Runtime/Containers/List.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <type_traits>

namespace Horizon::Engine
{
	class ReflectionSystem;

	class H_EXPORT World final
	{
	public:
		World(ReflectionSystem* pReflection) : m_components(pReflection)
		{
		}
		~World() = default;

		World(const World&) = delete;
		World& operator=(const World&) = delete;

		void BeginStructuralPhase() { m_structural = true; }
		void EndStructuralPhase() { m_structural = false; }
		b8 IsStructuralPhase() const { return m_structural; }

		EntityHandle CreateEntity();
		void DestroyEntity(EntityHandle entity);

		void CollectComponents(EntityHandle entity, List<ComponentObject*>& outComponents);

		template<typename T>
		T* AddComponent(EntityHandle entity, T&& component)
		{
			if (!m_entities.IsAlive(entity))
			{
				Terminal::Warn(StringOps::GetName(this), "AddComponent on a dead or stale entity");
				return nullptr;
			}

			ComponentStorage* pStorage = m_components.GetOrCreateStorage(Reflect::TypeOf<T>());
			if (!pStorage)
				return nullptr;

			void* pSlot = pStorage->InsertMoved(entity, &component);
			if (!pSlot)
				return nullptr;

			m_signatures[(u32)entity.Index()].set(pStorage->GetSlot());

			return static_cast<T*>(pSlot);
		}

		template<typename T>
		void RemoveComponent(EntityHandle entity)
		{
			ComponentStorage* pStorage = m_components.FindStorage(Reflect::TypeOf<T>());
			if (!pStorage)
				return;

			pStorage->Remove(entity);
			m_signatures[(u32)entity.Index()].reset(pStorage->GetSlot());
		}

		template<typename T>
		T* FindComponent(EntityHandle entity)
		{
			if (!m_entities.IsAlive(entity))
				return nullptr;

			ComponentStorage* pStorage = m_components.FindStorage(Reflect::TypeOf<T>());
			if (!pStorage)
				return nullptr;

			return static_cast<T*>(pStorage->Find(entity));
		}

		template<typename T>
		b8 HasComponent(EntityHandle entity) const
		{
			if (!m_entities.IsAlive(entity))
				return false;

			u32 slot = m_components.FindSlot(Reflect::TypeOf<T>());
			if (slot == InvalidSlot)
				return false;

			return m_signatures[(u32)entity.Index()].test(slot);
		}

		template<typename T, typename Fn>
		void ForEach(Fn&& callback)
		{
			ComponentStorage* pStorage = m_components.FindStorage(Reflect::TypeOf<T>());
			if (!pStorage)
				return;

			for (usize i = 0; i < pStorage->GetCount(); i++)
				callback(pStorage->GetEntityAt(i), *static_cast<T*>(pStorage->GetAt(i)));
		}

		template<typename TPivot, typename... TOthers, typename Fn>
		void ForView(Fn&& callback)
		{
			ComponentStorage* pPivot = m_components.FindStorage(Reflect::TypeOf<TPivot>());
			if (!pPivot)
				return;

			for (usize i = 0; i < pPivot->GetCount(); i++)
			{
				EntityHandle entity = pPivot->GetEntityAt(i);

				if ((HasComponent<TOthers>(entity) && ...))
					callback(entity, *static_cast<TPivot*>(pPivot->GetAt(i)), *FindComponent<TOthers>(entity)...);
			}
		}

		EntityStorage& GetEntities() { return m_entities; }
		ComponentRegistry& GetComponents() { return m_components; }
		const Signature& GetSignature(EntityHandle entity) const { return m_signatures[(u32)entity.Index()]; }

		void MarkComponent(EntityHandle entity, u32 slot) { m_signatures[(u32)entity.Index()].set(slot); }

	private:
		b8 EnsureStructural(std::string_view callName) const;

	private:
		EntityStorage m_entities;
		ComponentRegistry m_components;
		Signature m_signatures[MaxEntities];
		b8 m_structural = false;
	};
}