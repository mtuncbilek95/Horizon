#pragma once

#include <Engine/World/ECS/ComponentObject.h>
#include <Engine/World/ECS/Definitions.h>

#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class H_EXPORT IComponentStorage
	{
	public:
		virtual ~IComponentStorage() = default;

		virtual ComponentTypeId GetComponentTypeId() const = 0;
		virtual b8 Contains(EntityHandle entt) const = 0;
		virtual void Remove(EntityHandle entt) = 0;
		virtual usize GetCount() const = 0;
		virtual EntityHandle GetEntityAt(usize index) const = 0;

		u32 GetSlot() const { return m_slot; }
		void SetSlot(u32 slot) { m_slot = slot; }

	private:
		u32 m_slot = kInvalid32;
	};

	template<typename T>
	class H_EXPORT ComponentStorage final : public IComponentStorage
	{
	public:
		ComponentStorage()
		{
			// Minimum entity count.
			m_sparse.Resize(AtLeastEntities);

			for (usize i = 0; i < m_sparse.GetCount(); i++)
				m_sparse[i] = kInvalid32;
		}
		~ComponentStorage() = default;

		T& Insert(EntityHandle handl, T&& comp)
		{
			const u32 index = (u32)handl.Index();
			EnsureSparse(index); // Check if it needs resize.

			u32 denseIndex = m_sparse[index];
			if (denseIndex != kInvalid32)
			{
				Terminal::Warn(StringOps::GetName(this), "Entity {} already owns this component", index);
				return m_dense[denseIndex];
			}

			m_sparse[index] = (u32)m_dense.GetCount();
			m_entity.PushBack(handl);

			return m_dense.EmplaceBack(std::move(comp));
		}

		T* Find(EntityHandle handl)
		{
			u32 index = (u32)handl.Index();
			if (index >= m_sparse.GetCount())
				return nullptr;

			u32 denseIndex = m_sparse[index];
			if (denseIndex == kInvalid32)
				return nullptr;

			return &m_dense[denseIndex];
		}

		void Remove(EntityHandle handl) final
		{
			const u32 index = (u32)handl.Index();

			if (index >= m_sparse.GetCount() || m_sparse[index] == kInvalid32)
				return;

			const u32 removeAt = m_sparse[index];
			const u32 lastAt = (u32)m_dense.GetCount() - 1;

			if (removeAt != lastAt)
			{
				m_dense[removeAt] = std::move(m_dense[lastAt]);
				m_entity[removeAt] = m_entity[lastAt];
				m_sparse[(u32)m_entity[removeAt].Index()] = removeAt;
			}

			m_dense.PopBack();
			m_entity.PopBack();
			m_sparse[index] = kInvalid32;
		}

		b8 Contains(EntityHandle entity) const final
		{
			const u32 index = (u32)entity.Index();

			if (index >= m_sparse.GetCount())
				return false;

			return m_sparse[index] != kInvalid32;
		}

		T& GetAt(usize index) { return m_dense[index]; }
		ComponentTypeId GetComponentTypeId() const final { return Reflect::TypeOf<T>(); }
		usize GetCount() const final { return m_dense.GetCount(); }
		EntityHandle GetEntityAt(usize denseIndex) const final { return m_entity[denseIndex]; }

	private:
		void EnsureSparse(u32 index)
		{
			const usize oldCount = m_sparse.GetCount();

			if (index < oldCount)
				return;

			usize newCount = oldCount == 0 ? AtLeastEntities : oldCount;

			while (newCount <= index)
				newCount *= 2;

			m_sparse.Resize(newCount);

			for (usize i = oldCount; i < newCount; i++)
				m_sparse[i] = kInvalid32;
		}

	private:
		List<T> m_dense;
		List<EntityHandle> m_entity;
		List<u32> m_sparse;
	};
}