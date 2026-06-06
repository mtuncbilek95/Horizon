#pragma once

#include <Engine/ECS/Definitions.h>
#include <Engine/ECS/EntityRegistry.h>
#include <Engine/ECS/ComponentRegistry.h>

namespace Horizon
{
	class WorldRegistry
	{
	public:
		EntityHandle CreateEntity()
		{
			return m_entities.NewEntity();
		}

		void DestroyEntity(EntityHandle entityHandle)
		{
			m_entities.DestroyEntity(entityHandle);
			m_components.OnEntityDestroy(entityHandle);
			m_signatures[entityHandle.id].reset();
		}

		template<typename T>
		void AddComponent(EntityHandle entityHandle, T component)
		{
			if (!m_components.IsComponentRegistered<T>())
				m_components.RegisterComponent<T>();

			auto& componentArray = m_components.GetComponentArray<T>();
			componentArray.Insert(entityHandle, component);
			m_signatures[entityHandle.id].set(GetComponentTypeId<T>());
		}

		template<typename T>
		void RemoveComponent(EntityHandle entityHandle)
		{
			if (!m_components.IsComponentRegistered<T>())
				return;

			auto& componentArray = m_components.GetComponentArray<T>();
			componentArray.Remove(entityHandle);

			m_signatures[entityHandle.id].reset(GetComponentTypeId<T>());
		}

		template<typename T>
		T& GetComponent(EntityHandle entityHandle)
		{
			assert(m_components.IsComponentRegistered<T>() && "Component not registered");
			return m_components.GetComponentArray<T>().GetComponent(entityHandle);
		}

		template<typename T>
		bool HasComponent(EntityHandle entityHandle) const
		{
			assert(entityHandle.isValid() && "Entity is invalid");
			return m_signatures[entityHandle.id].test(GetComponentTypeId<T>());
		}

		const Signature& GetSignature(EntityHandle entityHandle) const
		{
			assert(entityHandle.isValid() && "Entity is invalid");
			return m_signatures[entityHandle.id];
		}

		template<typename T, typename Fn>
		void ForEach(Fn&& callback)
		{
			if (!m_components.IsComponentRegistered<T>())
				return;

			auto& componentArray = m_components.GetComponentArray<T>();
			for (u32 componentIdx = 0; componentIdx < componentArray.Count(); componentIdx++)
				callback(componentArray.EntityAt(componentIdx), componentArray.ElementAt(componentIdx));
		}

		template<typename T, typename... Filter, typename Fn>
		void ForView(Fn&& callback)
		{
			if (!m_components.IsComponentRegistered<T>())
				return;

			auto& componentArray = m_components.GetComponentArray<T>();

			for (u32 componentIdx = 0; componentIdx < componentArray.Count(); ++componentIdx)
			{
				EntityHandle entity = componentArray.EntityAt(componentIdx);

				if ((HasComponent<Filter>(entity) && ...))
					callback(entity, componentArray.ElementAt(componentIdx));
			}
		}

	private:
		EntityRegistry m_entities;
		ComponentRegistry m_components;
		std::array<Signature, MaxEntities> m_signatures;
	};
}
