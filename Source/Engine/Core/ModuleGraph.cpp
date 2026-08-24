#include "ModuleGraph.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::Engine
{
	b8 ModuleGraph::Resolve(const List<std::type_index>& nodes, const std::unordered_set<std::type_index>& satisfied, List<std::type_index>& outOrder) const
	{
		std::unordered_map<std::type_index, usize> pending;
		std::unordered_map<std::type_index, List<std::type_index>> dependents;

		for (const auto& node : nodes)
		{
			pending.try_emplace(node, 0);
			dependents.try_emplace(node);
		}

		std::unordered_map<std::type_index, List<std::type_index>> edges = m_edges;

		for (const auto& edge : m_reverseEdges)
		{
			if (satisfied.contains(edge.after))
			{
				Terminal::Warn(StringOps::GetName(this), "{} precedes {} but {} is already active, dropping the constraint", StringOps::GetNameString(edge.before.name()), StringOps::GetNameString(edge.after.name()), StringOps::GetNameString(edge.after.name()));
				continue;
			}

			if (!pending.contains(edge.after))
			{
				Terminal::Warn(StringOps::GetName(this), "{} precedes unregistered module {}, dropping the constraint", StringOps::GetNameString(edge.before.name()), StringOps::GetNameString(edge.after.name()));
				continue;
			}

			edges[edge.after].PushBack(edge.before);
		}

		for (const auto& node : nodes)
		{
			auto it = edges.find(node);

			if (it == edges.end())
				continue;

			for (const auto& dependency : it->second)
			{
				if (satisfied.contains(dependency))
					continue;

				if (!pending.contains(dependency))
				{
					Terminal::Error(StringOps::GetName(this), "{} depends on unregistered module {}", StringOps::GetNameString(node.name()), StringOps::GetNameString(dependency.name()));
					return false;
				}

				pending[node]++;
				dependents[dependency].PushBack(node);
			}
		}

		outOrder.Clear();
		outOrder.Reserve(nodes.GetCount());

		List<std::type_index> ready;

		for (const auto& node : nodes)
		{
			if (pending[node] == 0)
				ready.PushBack(node);
		}

		while (!ready.IsEmpty())
		{
			std::type_index node = ready.Back();
			ready.PopBack();

			outOrder.PushBack(node);

			for (const auto& dependent : dependents[node])
			{
				pending[dependent]--;

				if (pending[dependent] == 0)
					ready.PushBack(dependent);
			}
		}

		if (outOrder.GetCount() != nodes.GetCount())
		{
			Terminal::Fatal(StringOps::GetName(this), "Cyclic dependency detected inside the pending module batch");
			return false;
		}

		return true;
	}

	void ModuleGraph::Clear()
	{
		m_edges.clear();
		m_reverseEdges.Clear();
		m_current = typeid(void);
	}
}