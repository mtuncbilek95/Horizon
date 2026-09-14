#include "JobGraph.h"

#include <Runtime/Log/Terminal.h>
#include <Runtime/Containers/StringOps.h>

namespace Horizon::Engine
{
	GraphNodeId JobGraph::AddNode(JobLane lane, Job&& job)
	{
		if (!job.IsValid())
		{
			Terminal::Error(StringOps::GetName(this), "Rejected empty job on {} lane", u32(lane));
			return InvalidGraphNode;
		}

		// Add return and fill with peace
		CompiledGraphNode& node = m_nodes.EmplaceBack();
		node.lane = lane;
		node.job = std::move(job);

		return (GraphNodeId)(m_nodes.GetCount() - 1);
	}

	GraphNodeId JobGraph::AddAfter(GraphNodeId before, JobLane lane, Job&& job)
	{
		// Add
		GraphNodeId id = AddNode(lane, std::move(job));

		if (id == InvalidGraphNode)
			return InvalidGraphNode;

		// Handle the dependency like a big champ
		AddDependency(before, id);
		return id;
	}

	b8 JobGraph::AddDependency(GraphNodeId before, GraphNodeId after)
	{
		// before after or same fuckery check
		if (before >= m_nodes.GetCount() || after >= m_nodes.GetCount() || before == after)
		{
			Terminal::Error(StringOps::GetName(this), "Invalid dependency {} -> {} with {} nodes", before, after, m_nodes.GetCount());
			return false;
		}

		// Check if its already there
		if (m_nodes[before].successors.Contains(after))
			return true;

		// Do the thing and move the fuck on
		m_nodes[before].successors.PushBack(after);
		m_nodes[after].dependencyCount++;

		return true;
	}

	CompiledGraph JobGraph::Compile()
	{
		CompiledGraph result;

		List<u32> remaining(m_nodes.GetCount());
		List<GraphNodeId> ready;

		// Get to the chopper
		for (usize i = 0; i < m_nodes.GetCount(); i++)
		{
			remaining[i] = m_nodes[i].dependencyCount;

			if (remaining[i] == 0)
				ready.PushBack((GraphNodeId)i);
		}

		usize visited = 0;

		// design the dependency tree properly
		while (!ready.IsEmpty())
		{
			GraphNodeId id = ready.Back();
			ready.PopBack();

			visited++;

			for (GraphNodeId successor : m_nodes[id].successors)
			{
				if (--remaining[successor] == 0)
					ready.PushBack(successor);
			}
		}

		if (visited != m_nodes.GetCount())
		{
			Terminal::Error(StringOps::GetName(this), "Graph has a cycle, only {} of {} nodes are reachable", visited, m_nodes.GetCount());
			return result;
		}

		result.nodes = std::move(m_nodes);
		return result;
	}
}