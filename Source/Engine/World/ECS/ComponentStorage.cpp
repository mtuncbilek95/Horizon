#include "ComponentStorage.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	namespace
	{
		static constexpr usize sInitialCapacity = 16;
	}

	ComponentStorage::ComponentStorage(const Reflect::Type* pType) : m_type(pType)
	{
		m_sparse.Resize(AtLeastEntities);

		for (usize i = 0; i < m_sparse.GetCount(); i++)
			m_sparse[i] = kInvalid32;
	}

	ComponentStorage::~ComponentStorage()
	{
		for (usize i = 0; i < m_count; i++)
			m_type->DestructAt(SlotAt(i));

		if (m_data)
			Memory::Allocator::FreeRaw(m_data);
	}

	ComponentObject* ComponentStorage::Insert(EntityHandle handl, void* pSource)
	{
		if (!m_type->CanMove())
		{
			Terminal::Error(StringOps::GetName(this), "'{}' is not move constructible", m_type->GetName());
			return nullptr;
		}

		void* pSlot = AcquireSlot(handl);
		if (!pSlot)
			return Find(handl);

		m_type->MoveAt(pSlot, pSource);
		return static_cast<ComponentObject*>(pSlot);
	}

	ComponentObject* ComponentStorage::InsertBlind(EntityHandle handl)
	{
		if (!m_type->CanConstruct())
		{
			Terminal::Error(StringOps::GetName(this), "'{}' is not default constructible", m_type->GetName());
			return nullptr;
		}

		void* pSlot = AcquireSlot(handl);
		if (!pSlot)
			return Find(handl);

		m_type->ConstructAt(pSlot);
		return static_cast<ComponentObject*>(pSlot);
	}

	ComponentObject* ComponentStorage::Find(EntityHandle handl) const
	{
		const u32 index = (u32)handl.Index();
		if (index >= m_sparse.GetCount())
			return nullptr;

		const u32 denseIndex = m_sparse[index];
		if (denseIndex == kInvalid32)
			return nullptr;

		return GetAt(denseIndex);
	}

	void ComponentStorage::Remove(EntityHandle handl)
	{
		const u32 index = (u32)handl.Index();

		if (index >= m_sparse.GetCount() || m_sparse[index] == kInvalid32)
			return;

		const u32 removeAt = m_sparse[index];
		const u32 lastAt = (u32)m_count - 1;

		void* pRemove = SlotAt(removeAt);
		m_type->DestructAt(pRemove);

		if (removeAt != lastAt)
		{
			void* pLast = SlotAt(lastAt);
			m_type->MoveAt(pRemove, pLast);
			m_type->DestructAt(pLast);

			m_entity[removeAt] = m_entity[lastAt];
			m_sparse[(u32)m_entity[removeAt].Index()] = removeAt;
		}

		m_entity.PopBack();
		m_sparse[index] = kInvalid32;
		m_count--;
	}

	b8 ComponentStorage::Contains(EntityHandle handl) const
	{
		const u32 index = (u32)handl.Index();

		if (index >= m_sparse.GetCount())
			return false;

		return m_sparse[index] != kInvalid32;
	}

	ComponentObject* ComponentStorage::GetAt(usize denseIndex) const
	{
		return static_cast<ComponentObject*>(SlotAt(denseIndex));
	}

	void* ComponentStorage::SlotAt(usize denseIndex) const
	{
		return m_data + denseIndex * m_type->GetSizeInBytes();
	}

	void* ComponentStorage::AcquireSlot(EntityHandle handl)
	{
		const u32 index = (u32)handl.Index();
		EnsureSparse(index);

		if (m_sparse[index] != kInvalid32)
		{
			Terminal::Warn(StringOps::GetName(this), "Entity {} already owns '{}'", index, m_type->GetName());
			return nullptr;
		}

		EnsureCapacity(m_count + 1);

		m_sparse[index] = (u32)m_count;
		m_entity.PushBack(handl);

		void* pSlot = SlotAt(m_count);
		m_count++;

		return pSlot;
	}

	void ComponentStorage::EnsureCapacity(usize required)
	{
		if (required <= m_capacity)
			return;

		usize newCapacity = m_capacity == 0 ? sInitialCapacity : m_capacity;

		while (newCapacity < required)
			newCapacity *= 2;

		const usize stride = m_type->GetSizeInBytes();
		u8* pNewData = (u8*)Memory::Allocator::AllocateRaw(newCapacity * stride, m_type->GetAlignment(), Memory::CurrLoc());

		for (usize i = 0; i < m_count; i++)
		{
			void* pOld = SlotAt(i);
			void* pNew = pNewData + i * stride;

			m_type->MoveAt(pNew, pOld);
			m_type->DestructAt(pOld);
		}

		if (m_data)
			Memory::Allocator::FreeRaw(m_data);

		m_data = pNewData;
		m_capacity = newCapacity;
	}

	void ComponentStorage::EnsureSparse(u32 index)
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
}