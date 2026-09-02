#include "EntityStorage.h"

namespace Horizon::Engine
{
	EntityStorage::EntityStorage()
	{
		m_generations.Resize(AtLeastEntities);
		m_alive.Resize(AtLeastEntities);
		m_signatures.Resize(AtLeastEntities);
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
			index = m_nextIndex.Load();
			m_nextIndex.FetchAdd(1);

			EnsureIndex(index);
		}

		m_alive[index] = true;

		return EntityHandle::Generate(index, m_generations[index]);
	}

	b8 EntityStorage::Destroy(EntityHandle handl)
	{
		if (!IsAlive(handl))
		{
			Terminal::Warn(StringOps::GetName(this), "Destroy called on a dead or stale entity");
			return false;
		}

		const u32 index = handl.Index();

		m_alive[index] = false;
		m_signatures[index].reset();
		m_generations[index]++;
		m_freeList.PushBack(index);

		return true;
	}

	b8 EntityStorage::IsAlive(EntityHandle handl) const
	{
		if (!handl.IsValid())
			return false;

		const u32 index = u32(handl.Index());

		if (index >= m_alive.GetCount())
			return false;

		return m_alive[index] && m_generations[index] == u32(handl.Generation());
	}

	EntityHandle EntityStorage::GetHandleAt(u32 index) const
	{
		if (index >= m_alive.GetCount() || !m_alive[index])
			return EntityHandle();

		return EntityHandle::Generate(index, m_generations[index]);
	}

	Signature* EntityStorage::GetSignatureOf(EntityHandle handl)
	{
		u32 index = handl.Index();
		if (!handl.IsValid() || index >= m_signatures.GetCount())
			return nullptr;

		return &m_signatures[index];
	}

	const Signature* EntityStorage::GetSignatureOf(EntityHandle handl) const
	{
		u32 index = handl.Index();
		if (!handl.IsValid() || index >= m_signatures.GetCount())
			return nullptr;

		return &m_signatures[index];
	}

	u32 EntityStorage::GetAliveCount() const
	{
		return m_nextIndex.Load() - u32(m_freeList.GetCount());
	}

	void EntityStorage::EnsureIndex(u32 index)
	{
		if (index < m_generations.GetCount())
			return;

		usize newCount = m_generations.GetCount();

		if (newCount == 0)
			newCount = AtLeastEntities;

		while (newCount <= index)
			newCount *= 2;

		m_generations.Resize(newCount);
		m_alive.Resize(newCount);
		m_signatures.Resize(newCount);
	}
}