#pragma once

#include <Engine/Core/Engine.h>
#include <Engine/Job/JobSystem.h>
#include <Engine/World/SystemDependency.h>
#include <Engine/World/System.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/Containers/Memory/RefPtr.h>

#include <unordered_map>
#include <vector>

namespace Horizon::Engine
{
	class Scene;

	class SystemGraph final
	{
	public:
		b8 OnInititalize();
		void OnExecute(const EngineFrame& frame, Scene* pScene);
		void OnFinalize();

		System* FindSystem(Reflect::TypeHandle handle);

	private:
		void AddEdge(List<List<usize>>& adjacency, usize beforeIndex, usize afterIndex);
		List<List<usize>> BuildExplicitEdges(List<SystemDependency>& dependencyList);
		b8 TopologicalSort(List<usize>& order);
		void ReportCycle(const List<usize>& order);
		void BuildHazardEdges(List<usize>& order, List<SystemDependency>& dependencyList);

		void Schedule(usize index, const EngineFrame* pFrame);

	private:
		List<RefPtr<System>> m_activeSystems;
		std::unordered_map<Reflect::TypeHandle, usize> m_systemLookup;

		List<List<usize>> m_adjacency;
		List<usize> m_indegrees;

		std::vector<std::atomic<i64>> m_counters;
		PAL::Atomic<i64> m_remaining;

		b8 m_sorted = false;
	};
}