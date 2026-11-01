#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	class SystemDependency
	{
		friend class SystemGraph;
	public:
		template<typename... Ts>
		void Reads()
		{
			(m_reads.PushBack(Reflect::TypeOf<Ts>()), ...);
		}

		template<typename... Ts>
		void Writes()
		{
			(m_writes.PushBack(Reflect::TypeOf<Ts>()), ...);
		}

		template<typename... Ts>
		void RunsBefore()
		{
			(m_runsBefore.PushBack(Reflect::TypeOf<Ts>()), ...);
		}

		template<typename... Ts>
		void RunsAfter()
		{
			(m_runsAfter.PushBack(Reflect::TypeOf<Ts>()), ...);
		}

	private:
		List<Reflect::TypeHandle> m_reads;
		List<Reflect::TypeHandle> m_writes;

		List<Reflect::TypeHandle> m_runsBefore;
		List<Reflect::TypeHandle> m_runsAfter;
	};
}