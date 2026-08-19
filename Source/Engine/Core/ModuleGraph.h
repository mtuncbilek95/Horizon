#pragma once

#include <Runtime/Containers/List.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <typeindex>
#include <unordered_map>
#include <unordered_set>

namespace Horizon::Engine
{
	/*
	* ModuleGraph collects the dependency edges declared by the modules
	* waiting in Engine's pending list. Engine opens one node per pending
	* module, the module fills it through Requires<T>(), and Resolve turns
	* the batch into a topological initialization order. Types passed as
	* satisfied are already active modules, so their edges are dropped
	* instead of being ordered a second time.
	*/
	class H_EXPORT ModuleGraph
	{
	public:
		void BeginNode(std::type_index owner)
		{
			m_current = owner;
			m_edges.try_emplace(owner);
		}

		template<typename T>
		void Requires()
		{
			m_edges[m_current].PushBack(std::type_index(typeid(T)));
		}

		b8 Resolve(const List<std::type_index>& nodes, const std::unordered_set<std::type_index>& satisfied, List<std::type_index>& outOrder) const;
		void Clear();

	private:
		std::unordered_map<std::type_index, List<std::type_index>> m_edges;
		std::type_index m_current = typeid(void);
	};
}