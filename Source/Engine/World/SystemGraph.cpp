#include "SystemGraph.h"

#include <Engine/Job/JobSystem.h>
#include <Runtime/Containers/StringHelper.h>

#include <vector>
#include <queue>

namespace Horizon::Engine
{
	b8 SystemGraph::OnInititalize()
	{
		auto* pReflection = m_engine->GetReflectionSystem();
		
		List<Reflect::Type*> systemTypes = pReflection->GetTypeByBase(Reflect::TypeOf<System>());
		for (auto* pType : systemTypes)
		{
			RefPtr<System> pSystem = MakeRef<System>(pType);
			if (!pSystem)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} may have missing v-table implementation. Check your virtual functions.", pType->GetName());
				continue;
			}

			pSystem->m_engine = m_engine;

			if (!pSystem->OnInitialize())
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} has failed initializing. Check out OnInitialize to find the issue.", pType->GetName());
				continue;
			}

			m_systemLookup[pType->GetTypeId()] = m_activeSystems.GetCount();
			m_activeSystems.PushBack(pSystem);

			Terminal::Info(StringHelper::GetTypeName(this), "{} is ready to be used. Registered successfully.", pType->GetName());
		}

		List<SystemDependency> dependencyList(m_activeSystems.GetCount());
		for (usize i = 0; i < m_activeSystems.GetCount(); i++)
			m_activeSystems[i]->DeclareDependencies(dependencyList[i]);

		m_adjacency = BuildExplicitEdges(dependencyList);

		List<usize> order;
		if (!TopologicalSort(order))
		{
			ReportCycle(order);
			m_adjacency.Clear();
			return m_sorted = false;
		}

		BuildHazardEdges(order, dependencyList);

		m_indegrees = List<usize>(m_adjacency.GetCount(), 0);
		for (usize i = 0; i < m_adjacency.GetCount(); i++)
		{
			for (usize next : m_adjacency[i])
				m_indegrees[next]++;
		}

		m_counters = List<i64>(m_adjacency.GetCount());
		return m_sorted = true;
	}

	void SystemGraph::OnExecute(const EngineFrame& frame, Scene* pScene)
	{
		if (!m_sorted)
			return;

		for (usize i = 0; i < m_indegrees.GetCount(); i++)
			PAL::AtomicOps::FetchStore(&m_counters[i], i64(m_indegrees[i]));

		m_remaining.Store(i64(m_activeSystems.GetCount()));

		for (usize i = 0; i < m_indegrees.GetCount(); i++)
		{
			if (m_indegrees[i] == 0)
				Schedule(i, &frame, pScene);
		}

		i64 remaining = 0;
		while ((remaining = m_remaining.Load()) != 0)
			PAL::Futex::Wait(m_remaining.Address(), remaining);
	}

	void SystemGraph::OnFinalize()
	{
		i64 remaining = 0;
		while ((remaining = m_remaining.Load()) != 0)
			PAL::Futex::Wait(m_remaining.Address(), remaining);

		for (usize i = m_activeSystems.GetCount(); i > 0; --i)
			m_activeSystems[i - 1]->OnFinalize();

		for (usize i = m_activeSystems.GetCount(); i > 0; --i)
		{
			m_activeSystems[i - 1].Reset();
		}

		m_activeSystems.Clear();
		m_systemLookup.clear();

		m_adjacency.Clear();
		m_indegrees.Clear();
		m_counters.Clear();
		m_sorted = false;
	}

	System* SystemGraph::FindSystem(Reflect::TypeHandle handle)
	{
		auto it = m_systemLookup.find(handle);
		if (it == m_systemLookup.end())
			return nullptr;

		return m_activeSystems[it->second].Get();
	}

	void SystemGraph::AddEdge(List<List<usize>>& adjacency, usize beforeIndex, usize afterIndex)
	{
		adjacency[beforeIndex].PushBack(afterIndex);
	}

	List<List<usize>> SystemGraph::BuildExplicitEdges(List<SystemDependency>& dependencyList)
	{
		List<List<usize>> adjacency(m_activeSystems.GetCount());
		for (usize i = 0; i < m_activeSystems.GetCount(); i++)
		{
			SystemDependency& depend = dependencyList[i];

			for (const auto& index : depend.m_runsBefore)
				AddEdge(adjacency, i, m_systemLookup.at(index));

			for (const auto& index : depend.m_runsAfter)
				AddEdge(adjacency, m_systemLookup.at(index), i);
		}

		return adjacency;
	}

	b8 SystemGraph::TopologicalSort(List<usize>& order)
	{
		List<usize> indegrees(m_adjacency.GetCount(), 0);
		std::priority_queue<usize, std::vector<usize>, std::greater<>> systemQ;

		for (usize i = 0; i < m_adjacency.GetCount(); i++)
		{
			for (usize next : m_adjacency[i])
				indegrees[next]++;
		}

		// Get the first independent ones
		for (usize i = 0; i < m_adjacency.GetCount(); i++)
		{
			if (indegrees[i] == 0)
				systemQ.push(i);
		}

		// Resolve the rest
		while (!systemQ.empty())
		{
			usize idx = systemQ.top();
			systemQ.pop();

			order.PushBack(idx);

			for (usize next : m_adjacency[idx])
			{
				indegrees[next]--;

				if (indegrees[next] == 0)
					systemQ.push(next);
			}
		}

		return m_adjacency.GetCount() == order.GetCount();
	}

	void SystemGraph::ReportCycle(const List<usize>& order)
	{
		Terminal::Error(StringHelper::GetTypeName(this), "Cycle ! ! ! ! !");
	}

	void SystemGraph::BuildHazardEdges(List<usize>& order, List<SystemDependency>& dependencyList)
	{
		struct ResourceState
		{
			usize lastWriter = u64_max;
			List<usize> readers;
		};

		std::unordered_map<Reflect::TypeHandle, ResourceState> statesByType;

		for (usize sys : order)
		{
			const SystemDependency& dep = dependencyList[sys];

			for (Reflect::TypeHandle id : dep.m_reads)
			{
				ResourceState& state = statesByType[id];
				if (state.lastWriter != u64_max)
					AddEdge(m_adjacency, state.lastWriter, sys); // Read after Write

				state.readers.PushBack(sys);
			}

			for (Reflect::TypeHandle id : dep.m_writes)
			{
				ResourceState& state = statesByType[id];

				if (!state.readers.IsEmpty())
				{
					for (size_t reader : state.readers)
					{
						if (reader != sys)
							AddEdge(m_adjacency, reader, sys); // Write after Read
					}
				}
				else if (state.lastWriter != u64_max)
					AddEdge(m_adjacency, state.lastWriter, sys); // Write after Write

				state.lastWriter = sys;
				state.readers.Clear();
			}
		}
	}

	void SystemGraph::Schedule(usize index, const EngineFrame* pFrame, Scene* pScene)
	{
		m_engine->GetJobSystem()->SubmitDetached(JobLane::Critical, [this, index, pFrame, pScene]()
			{
				m_activeSystems[index]->OnExecute(*pFrame, *pScene);

				for (usize next : m_adjacency[index])
				{
					if (PAL::AtomicOps::Subtract(&m_counters[next], 1) == 0)
						Schedule(next, pFrame, pScene);
				}

				if (m_remaining.Subtract(1) == 0)
					PAL::Futex::WakeAll(m_remaining.Address());
			});
	}
}