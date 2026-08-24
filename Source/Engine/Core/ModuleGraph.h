#pragma once

#include <Runtime/Containers/List.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <typeindex>
#include <unordered_map>
#include <unordered_set>

namespace Horizon::Engine
{
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

		template<typename T>
		void Precedes()
		{
			m_reverseEdges.PushBack(ReverseEdge{ std::type_index(typeid(T)), m_current });
		}

		b8 Resolve(const List<std::type_index>& nodes, const std::unordered_set<std::type_index>& satisfied, List<std::type_index>& outOrder) const;
		void Clear();

	private:
		struct ReverseEdge
		{
			std::type_index after;
			std::type_index before;
		};

		std::unordered_map<std::type_index, List<std::type_index>> m_edges;
		List<ReverseEdge> m_reverseEdges;
		std::type_index m_current = typeid(void);
	};
}