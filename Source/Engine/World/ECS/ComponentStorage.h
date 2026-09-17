#pragma once

#include <Engine/World/ECS/ComponentObject.h>
#include <Engine/World/ECS/Definitions.h>

#include <Runtime/Containers/List.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class H_EXPORT ComponentStorage final
	{
	public:
		ComponentStorage(const Reflect::Type* pType);
		~ComponentStorage();

		ComponentStorage(const ComponentStorage&) = delete;
		ComponentStorage& operator=(const ComponentStorage&) = delete;

		ComponentObject* Insert(EntityHandle handl, void* pSource);
		ComponentObject* InsertBlind(EntityHandle handl);
		ComponentObject* Find(EntityHandle handl) const;
		void Remove(EntityHandle handl);
		b8 Contains(EntityHandle handl) const;

		ComponentObject* GetAt(usize denseIndex) const;
		EntityHandle GetEntityAt(usize denseIndex) const { return m_entity[denseIndex]; }
		usize GetCount() const { return m_count; }

		const Reflect::Type* GetType() const { return m_type; }
		ComponentTypeId GetComponentTypeId() const { return m_type->GetTypeId(); }

		u32 GetSlot() const { return m_slot; }
		void SetSlot(u32 slot) { m_slot = slot; }

	private:
		void* SlotAt(usize denseIndex) const;
		void* AcquireSlot(EntityHandle handl);
		void EnsureCapacity(usize required);
		void EnsureSparse(u32 index);

	private:
		const Reflect::Type* m_type = nullptr;
		u8* m_data = nullptr;
		usize m_count = 0;
		usize m_capacity = 0;

		List<EntityHandle> m_entity;
		List<u32> m_sparse;

		u32 m_slot = kInvalid32;
	};
}