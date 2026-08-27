#include "ComponentStorage.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	ComponentStorage::ComponentStorage(const Reflect::Type& type) : m_type(&type),
		m_stride(type.GetSizeInBytes())
	{
		m_sparse.Resize(MaxEntities);

		for (usize i = 0; i < m_sparse.GetCount(); i++)
			m_sparse[i] = InvalidDenseIndex;
	}

	ComponentStorage::~ComponentStorage()
	{
		for (usize i = 0; i < m_count; i++)
			m_type->DestructAt(m_data + i * m_stride);

		if (m_data)
			Memory::Allocator::FreeRaw(m_data);
	}

	void ComponentStorage::Reserve(usize capacity)
	{
		if (capacity <= m_capacity)
			return;

		u8* pBuffer = static_cast<u8*>(Memory::Allocator::AllocateRaw(capacity * m_stride, m_type->GetAlignment(), Memory::CurrLoc()));

		if (!pBuffer)
		{
			Terminal::Error(StringOps::GetName(this), "'{}' could not grow to {} elements", m_type->GetName(), capacity);
			return;
		}

		for (usize i = 0; i < m_count; i++)
		{
			m_type->MoveAt(pBuffer + i * m_stride, m_data + i * m_stride);
			m_type->DestructAt(m_data + i * m_stride);
		}

		if (m_data)
			Memory::Allocator::FreeRaw(m_data);

		m_data = pBuffer;
		m_capacity = capacity;
	}

	void* ComponentStorage::Emplace(EntityHandle entity)
	{
		u32 index = (u32)entity.Index();

		if (index >= MaxEntities)
		{
			Terminal::Error(StringOps::GetName(this), "Entity index {} is outside the budget of {}", index, MaxEntities);
			return nullptr;
		}

		if (m_sparse[index] != InvalidDenseIndex)
		{
			Terminal::Warn(StringOps::GetName(this), "Entity {} already owns a '{}'", index, m_type->GetName());
			return nullptr;
		}

		if (m_count == m_capacity)
			Reserve(m_capacity == 0 ? 2 : m_capacity * 2);

		if (m_count == m_capacity)
			return nullptr;

		m_sparse[index] = (u32)m_count;
		m_denseToEntity.PushBack(entity);

		void* pSlot = m_data + m_count * m_stride;
		m_count++;

		return pSlot;
	}

	void* ComponentStorage::Insert(EntityHandle entity)
	{
		void* pSlot = Emplace(entity);

		if (!pSlot)
			return Find(entity);

		m_type->ConstructAt(pSlot);
		return pSlot;
	}

	void* ComponentStorage::InsertMoved(EntityHandle entity, void* pSource)
	{
		void* pSlot = Emplace(entity);

		if (!pSlot)
			return Find(entity);

		m_type->MoveAt(pSlot, pSource);
		return pSlot;
	}

	void* ComponentStorage::Find(EntityHandle entity)
	{
		u32 index = (u32)entity.Index();

		if (index >= MaxEntities)
			return nullptr;

		u32 denseIndex = m_sparse[index];

		if (denseIndex == InvalidDenseIndex)
			return nullptr;

		return m_data + denseIndex * m_stride;
	}

	void ComponentStorage::Remove(EntityHandle entity)
	{
		u32 index = (u32)entity.Index();

		if (index >= MaxEntities || m_sparse[index] == InvalidDenseIndex)
			return;

		usize removeAt = m_sparse[index];
		usize lastAt = m_count - 1;

		m_type->DestructAt(m_data + removeAt * m_stride);

		if (removeAt != lastAt)
		{
			m_type->MoveAt(m_data + removeAt * m_stride, m_data + lastAt * m_stride);
			m_type->DestructAt(m_data + lastAt * m_stride);

			m_denseToEntity[removeAt] = m_denseToEntity[lastAt];
			m_sparse[(u32)m_denseToEntity[removeAt].Index()] = (u32)removeAt;
		}

		m_denseToEntity.PopBack();
		m_sparse[index] = InvalidDenseIndex;
		m_count--;
	}

	void ComponentStorage::ResizeDense(usize count)
	{
		if (count > m_capacity)
			Reserve(count);

		if (count > m_capacity)
			return;

		for (usize i = m_count; i < count; i++)
			m_type->ConstructAt(m_data + i * m_stride);

		for (usize i = count; i < m_count; i++)
		{
			m_type->DestructAt(m_data + i * m_stride);
			m_sparse[(u32)m_denseToEntity[i].Index()] = InvalidDenseIndex;
		}

		m_denseToEntity.Resize(count);
		m_count = count;
	}

	void ComponentStorage::BindEntityAt(usize denseIndex, EntityHandle entity)
	{
		u32 index = (u32)entity.Index();

		if (denseIndex >= m_count)
		{
			Terminal::Error(StringOps::GetName(this), "Dense index {} is outside the {} element range", denseIndex, m_count);
			return;
		}

		if (index >= MaxEntities)
		{
			Terminal::Error(StringOps::GetName(this), "Entity index {} is outside the budget of {}", index, MaxEntities);
			return;
		}

		m_denseToEntity[denseIndex] = entity;
		m_sparse[index] = (u32)denseIndex;
	}

	b8 ComponentStorage::Contains(EntityHandle entity) const
	{
		u32 index = (u32)entity.Index();

		if (index >= MaxEntities)
			return false;

		return m_sparse[index] != InvalidDenseIndex;
	}
}