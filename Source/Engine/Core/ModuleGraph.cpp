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

		for (const auto& node : nodes)
		{
			auto it = m_edges.find(node);

			if (it == m_edges.end())
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
		m_current = typeid(void);
	}
}