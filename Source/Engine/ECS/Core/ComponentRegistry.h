#pragma once

#include <Engine/ECS/Core/Definitions.h>

namespace Horizon
{
	class IComponentArray
	{
	public:
		virtual ~IComponentArray() = default;

		virtual void OnEntityDestroyed(EntityHandle handle) = 0;
	};

	template<typename T>
	class ComponentArray : public IComponentArray
	{
	public:
		ComponentArray() { m_sparse.fill(kInvalid32); }
		~ComponentArray() = default;

		void Insert(EntityHandle entity, T component)
		{
			assert(m_sparse[entity.id] == kInvalid32 && "Already registered entity for the component");

			m_dense[m_size] = component;
			m_denseToEntity[m_size] = entity;
			m_sparse[entity.id] = m_size;
			m_size++;
		}

		T& GetComponent(EntityHandle entity)
		{
			assert(m_sparse[entity.id] != kInvalid32 && "Invalid entity");

			u32 denseIndex = m_sparse[entity.id];
			return m_dense[denseIndex];
		}

		void Remove(EntityHandle entity)
		{
			assert(m_size > 0 && "ComponentArray is empty");
			assert(m_sparse[entity.id] != kInvalid32 && "Entity does not have this component");

			auto removeIndex = m_sparse[entity.id];
			auto lastIndex = m_size - 1;

			m_dense[removeIndex] = m_dense[lastIndex];
			m_dense[lastIndex] = T();
			m_size--;

			EntityHandle lastEntity = m_denseToEntity[lastIndex];
			m_sparse[lastEntity.id] = removeIndex;
			m_denseToEntity[removeIndex] = m_denseToEntity[lastIndex];
			m_sparse[entity.id] = kInvalid32;
		}

		void OnEntityDestroyed(EntityHandle handle) final
		{
			if (HasEntity(handle))
				Remove(handle);
		}

		b8 HasEntity(EntityHandle entity) const { return m_sparse[entity.id] != kInvalid32; }
		u32 Count() const { return m_size; }
		T& ElementAt(u32 index) { return m_dense[index]; }
		EntityHandle EntityAt(u32 index) const { return m_denseToEntity[index]; }

	private:
		std::array<T, MaxEntities> m_dense;
		std::array<EntityHandle, MaxEntities> m_denseToEntity;
		std::array<u32, MaxEntities> m_sparse;
		u32 m_size = 0;
	};

	class ComponentRegistry
	{
	public:
		ComponentRegistry() { m_components.fill(nullptr); }
		~ComponentRegistry()
		{
			for (auto* componentArray : m_components)
				delete componentArray;
		}
		template<typename T>
		void RegisterComponent()
		{
			assert(!m_components[GetComponentTypeId<T>()] && "Already registered component");

			auto index = GetComponentTypeId<T>();
			m_components[index] = new ComponentArray<T>();
		}

		template<typename T>
		b8 IsComponentRegistered()
		{
			auto index = GetComponentTypeId<T>();
			return m_components[index];
		}

		template<typename T>
		ComponentArray<T>& GetComponentArray()
		{
			assert(m_components[GetComponentTypeId<T>()] && "Non registered component");

			auto index = GetComponentTypeId<T>();
			return *static_cast<ComponentArray<T>*>(m_components[index]);
		}

		template<typename T>
		b8 HasComponentArray()
		{
			assert(m_components[GetComponentTypeId<T>()] && "Non registered component");

			auto index = GetComponentTypeId<T>();
			return m_components[index];
		}

		void OnEntityDestroy(EntityHandle entityHandle)
		{
			for (auto* componentArray : m_components)
			{
				if (componentArray)
					componentArray->OnEntityDestroyed(entityHandle);
			}
		}

	private:
		std::array<IComponentArray*, MaxComponents> m_components;
	};
}
