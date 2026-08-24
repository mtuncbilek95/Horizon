#pragma once

#include <Engine/World/ComponentObject.h>
#include <Engine/World/Definitions.h>

#include <Runtime/Containers/List.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <type_traits>

namespace Horizon::Engine
{
	class H_EXPORT IComponentStorage
	{
	public:
		virtual ~IComponentStorage() = default;

		virtual ComponentTypeId GetComponentTypeId() const = 0;
		virtual b8 Contains(EntityHandle entity) const = 0;
		virtual void Remove(EntityHandle entity) = 0;
		virtual usize GetCount() const = 0;
		virtual EntityHandle GetEntityAt(usize denseIndex) const = 0;
		virtual ComponentObject* FindErased(EntityHandle entity) = 0;
		virtual ComponentObject* GetErasedAt(usize denseIndex) = 0;

		u32 GetSlot() const { return m_slot; }
		void SetSlot(u32 slot) { m_slot = slot; }

	private:
		u32 m_slot = InvalidSlot;
	};

	template<typename T>
	class H_EXPORT ComponentStorage final : public IComponentStorage
	{
		static_assert(std::is_base_of_v<ComponentObject, T>, "A component must derive from ComponentObject.");

	public:
		ComponentStorage()
		{
			m_sparse.Resize(MaxEntities);

			for (usize i = 0; i < m_sparse.GetCount(); i++)
				m_sparse[i] = InvalidDenseIndex;
		}
		~ComponentStorage() = default;

		T& Insert(EntityHandle entity, T&& component)
		{
			u32 index = (u32)entity.Index();
			u32 denseIndex = m_sparse[index];

			if (denseIndex != InvalidDenseIndex)
			{
				Terminal::Warn(StringOps::GetName(this), "Entity {} already owns this component", index);
				return m_dense[denseIndex];
			}

			m_sparse[index] = (u32)m_dense.GetCount();
			m_denseToEntity.PushBack(entity);

			return m_dense.EmplaceBack(std::move(component));
		}

		T* Find(EntityHandle entity)
		{
			u32 index = (u32)entity.Index();
			if (index >= MaxEntities)
				return nullptr;

			u32 denseIndex = m_sparse[index];
			if (denseIndex == InvalidDenseIndex)
				return nullptr;

			return &m_dense[denseIndex];
		}

		void Remove(EntityHandle entity) final
		{
			u32 index = (u32)entity.Index();
			if (index >= MaxEntities || m_sparse[index] == InvalidDenseIndex)
				return;

			u32 removeAt = m_sparse[index];
			u32 lastAt = (u32)m_dense.GetCount() - 1;

			if (removeAt != lastAt)
			{
				m_dense[removeAt] = std::move(m_dense[lastAt]);
				m_denseToEntity[removeAt] = m_denseToEntity[lastAt];
				m_sparse[(u32)m_denseToEntity[removeAt].Index()] = removeAt;
			}

			m_dense.PopBack();
			m_denseToEntity.PopBack();
			m_sparse[index] = InvalidDenseIndex;
		}

		ComponentTypeId GetComponentTypeId() const final { return Reflect::TypeOf<T>(); }
		b8 Contains(EntityHandle entity) const final { return (u32)entity.Index() < MaxEntities && m_sparse[(u32)entity.Index()] != InvalidDenseIndex; }
		usize GetCount() const final { return m_dense.GetCount(); }
		EntityHandle GetEntityAt(usize denseIndex) const final { return m_denseToEntity[denseIndex]; }
		ComponentObject* FindErased(EntityHandle entity) final { return Find(entity); }
		ComponentObject* GetErasedAt(usize denseIndex) final { return &m_dense[denseIndex]; }

		T& GetAt(usize denseIndex) { return m_dense[denseIndex]; }
		T* GetData() { return m_dense.GetData(); }

	private:
		List<T> m_dense;
		List<EntityHandle> m_denseToEntity;
		List<u32> m_sparse;
	};
}