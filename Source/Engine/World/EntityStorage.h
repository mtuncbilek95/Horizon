#pragma once

#include <Engine/World/Definitions.h>

#include <Runtime/Containers/List.h>
#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/Sync/Atomic.h>

namespace Horizon::Engine
{
	class EntityStorage final
	{
	public:
		EntityStorage();
		~EntityStorage() = default;

		EntityHandle Create();
		EntityHandle Reserve();
		b8 ReserveRange(u32 count, List<EntityHandle>& outHandles);

		void Commit(EntityHandle entity);
		b8 Destroy(EntityHandle entity);
		b8 IsAlive(EntityHandle entity) const;
		EntityHandle GetHandleAt(u32 index) const;

		u32 GetAliveCount() const { return m_nextIndex.Load() - (u32)m_freeList.GetCount(); }
		u32 GetHighWaterMark() const { return m_nextIndex.Load(); }

	private:
		List<u32> m_generations;
		List<b8> m_alive;
		List<u32> m_freeList;
		PAL::Atomic<u32> m_nextIndex = 0;
	};
}