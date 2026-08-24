#include "EntityStorage.h"

namespace Horizon::Engine
{
	EntityStorage::EntityStorage()
	{
		m_generations.Resize(MaxEntities);
		m_alive.Resize(MaxEntities);
	}

	EntityHandle EntityStorage::Create()
	{
		u32 index = 0;

		if (!m_freeList.IsEmpty())
		{
			index = m_freeList.Back();
			m_freeList.PopBack();
		}
		else
		{
			EntityHandle reserved = Reserve();
			if (!reserved.IsValid())
				return reserved;

			Commit(reserved);
			return reserved;
		}

		m_alive[index] = true;
		return EntityHandle::Generate(index, m_generations[index]);
	}

	EntityHandle EntityStorage::Reserve()
	{
		u32 index = m_nextIndex.FetchAdd(1);

		if (index >= MaxEntities)
		{
			m_nextIndex.Store(MaxEntities);
			Terminal::Error(StringOps::GetName(this), "Entity budget of {} is exhausted", MaxEntities);
			return EntityHandle();
		}

		return EntityHandle::Generate(index, m_generations[index]);
	}

	void EntityStorage::Commit(EntityHandle entity)
	{
		if (!entity.IsValid())
			return;

		m_alive[(u32)entity.Index()] = true;
	}

	b8 EntityStorage::Destroy(EntityHandle entity)
	{
		if (!IsAlive(entity))
		{
			Terminal::Warn(StringOps::GetName(this), "Destroy called on a dead or stale entity");
			return false;
		}

		u32 index = (u32)entity.Index();
		m_alive[index] = false;
		m_generations[index]++;
		m_freeList.PushBack(index);

		return true;
	}

	b8 EntityStorage::IsAlive(EntityHandle entity) const
	{
		if (!entity.IsValid())
			return false;

		u32 index = (u32)entity.Index();
		if (index >= MaxEntities)
			return false;

		return m_alive[index] && m_generations[index] == (u32)entity.Generation();
	}

	EntityHandle EntityStorage::GetHandleAt(u32 index) const
	{
		if (index >= MaxEntities || !m_alive[index])
			return EntityHandle();

		return EntityHandle::Generate(index, m_generations[index]);
	}

}