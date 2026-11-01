#include "SystemGraph.h"

namespace Horizon::Engine
{
	b8 SystemGraph::OnInititalize()
	{
		return true;
	}

	void SystemGraph::OnExecute(const EngineFrame& frame, Scene* pScene)
	{
	}

	void SystemGraph::OnFinalize()
	{
	}

	System* SystemGraph::FindSystem(Reflect::TypeHandle handle)
	{
		return nullptr;
	}

	void SystemGraph::AddEdge(List<List<usize>>& adjacency, usize beforeIndex, usize afterIndex)
	{
	}

	List<List<usize>> SystemGraph::BuildExplicitEdges(List<SystemDependency>& dependencyList)
	{
		return List<List<usize>>();
	}

	b8 SystemGraph::TopologicalSort(List<usize>& order)
	{
		return true;
	}

	void SystemGraph::ReportCycle(const List<usize>& order)
	{
	}

	void SystemGraph::BuildHazardEdges(List<usize>& order, List<SystemDependency>& dependencyList)
	{
	}

	void SystemGraph::Schedule(usize index, const EngineFrame* pFrame)
	{
	}
}