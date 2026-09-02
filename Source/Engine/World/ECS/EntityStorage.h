#pragma once

#include <Engine/World/ECS/Definitions.h>

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

		b8 Destroy(EntityHandle handl);
		b8 IsAlive(EntityHandle handl) const;
		EntityHandle GetHandleAt(u32 index) const;

		Signature* GetSignatureOf(EntityHandle handl);
		const Signature* GetSignatureOf(EntityHandle handl) const;

		u32 GetAliveCount() const;
		u32 GetLatestIndex() const { return m_nextIndex.Load(); }

	private:
		void EnsureIndex(u32 index);

	private:
		List<u32> m_generations;
		List<b8> m_alive;
		List<u32> m_freeList;
		List<Signature> m_signatures;
		PAL::Atomic<u32> m_nextIndex = 0;
	};
}