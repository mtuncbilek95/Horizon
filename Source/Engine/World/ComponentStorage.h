#pragma once

#include <Engine/World/ComponentObject.h>
#include <Engine/World/Definitions.h>

#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class H_EXPORT ComponentStorage final
	{
	public:
		ComponentStorage(const Reflect::Type& type);
		~ComponentStorage();

		ComponentStorage(const ComponentStorage&) = delete;
		ComponentStorage& operator=(const ComponentStorage&) = delete;

		void* Insert(EntityHandle entity);
		void* InsertMoved(EntityHandle entity, void* pSource);
		void* Find(EntityHandle entity);
		void Remove(EntityHandle entity);

		void Reserve(usize capacity);
		void ResizeDense(usize count);
		void BindEntityAt(usize denseIndex, EntityHandle entity);

		b8 Contains(EntityHandle entity) const;

		void* GetAt(usize denseIndex) { return m_data + denseIndex * m_stride; }
		void* GetDenseData() { return m_data; }
		EntityHandle GetEntityAt(usize denseIndex) const { return m_denseToEntity[denseIndex]; }

		usize GetCount() const { return m_count; }
		usize GetStride() const { return m_stride; }

		const Reflect::Type& GetType() const { return *m_type; }
		ComponentTypeId GetComponentTypeId() const { return m_type->GetTypeId(); }

		u32 GetSlot() const { return m_slot; }
		void SetSlot(u32 slot) { m_slot = slot; }

	private:
		void* Emplace(EntityHandle entity);

	private:
		const Reflect::Type* m_type = nullptr;
		usize m_stride = 0;
		u8* m_data = nullptr;
		usize m_count = 0;
		usize m_capacity = 0;
		List<EntityHandle> m_denseToEntity;
		List<u32> m_sparse;
		u32 m_slot = InvalidSlot;
	};
}