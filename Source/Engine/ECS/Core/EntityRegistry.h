#pragma once

#include <Engine/ECS/Core/Definitions.h>

namespace Horizon
{
	class EntityRegistry
	{
	public:
		EntityRegistry()
		{
			m_alive.fill(false);
			m_generations.fill(0);
		}

		EntityHandle NewEntity()
		{
			u32 index;
			if (m_freeCount > 0)
			{
				m_freeCount--;
				index = m_freeList[m_freeCount];
			}
			else
			{
				assert(m_nextId < MaxEntities && "MaxEntities reached");
				index = m_nextId++;
			}

			m_alive[index] = true;
			return EntityHandle::Generate(index, m_generations[index]);
		}

		void DestroyEntity(EntityHandle handle)
		{
			assert(IsAlive(handle) && "Destroying dead/stale entity");

			u32 index = handle.Index();
			m_alive[index] = false;
			m_generations[index]++;

			assert(m_freeCount < MaxEntities && "FreeList full");

			m_freeList[m_freeCount++] = index;
		}

		b8 IsAlive(EntityHandle handle) const
		{
			if (!handle.IsValid())
				return false;

			u32 index = handle.Index();
			if (index >= MaxEntities)
				return false;

			return m_alive[index] && m_generations[index] == handle.Generation();
		}

	private:
		EntityId m_nextId = 0;
		u32 m_freeCount = 0;
		std::array<u32, MaxEntities> m_freeList;
		std::array<b8, MaxEntities> m_alive;
		std::array<u32, MaxEntities> m_generations;
	};
}
