#pragma once

#include <Engine/Core/Engine.h>
#include <Engine/Job/JobSystem.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/Rendering/RenderGraph/RenderNode.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Containers/Memory/RefPtr.h>

namespace Horizon::Engine
{
	class ENGINE_API RenderGraph
	{
	public:
		RenderGraph(Engine* pEngine);
		~RenderGraph();

		b8 OnInitialize();
		void OnExecute();
		void OnFinalize();

		void OnLibraryRegistered(const ReflectionLibrary& library);
		void OnLibraryUnregistered(const ReflectionLibrary& library);

	private:
		void AddEdge(List<List<usize>>& adjacency, usize beforeIndex, usize afterIndex);

		b8 CompileGraph(List<RenderDependency>& dependencyList);
		b8 ResolveResources(List<RenderDependency>& dependencyList);

		List<List<usize>> BuildExplicitEdges(List<RenderDependency>& dependencyList);
		b8 TopologicalSort(List<usize>& order);
		void ReportCycle(List<usize>& order);

		b8 ValidateAmbiguity(List<usize>& order, List<RenderDependency>& dependencyList);
		void BuildLifetimes(const List<usize>& order, List<RenderDependency>& dependencyList);
		b8 BuildBarriers(const List<usize>& order, List<RenderDependency>& dependencyList);

	private:
		Engine* m_engine = nullptr;
		ReflectionSystem* m_reflection = nullptr;
		JobSystem* m_job = nullptr;

		List<RefPtr<RenderNode>> m_activeNodes;
		std::unordered_map<Reflect::TypeHandle, usize> m_nodeLookup;

		std::unordered_map<Reflect::TypeHandle, usize> m_creatorLookup; // Which indexed node called Creates<>()...
		List<List<usize>> m_adjacency;

		List<usize> m_order;
	};
}