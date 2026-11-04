#pragma once

#include <Engine/Core/Engine.h>
#include <Engine/Job/JobSystem.h>
#include <Engine/Reflection/ReflectionSystem.h>
#include <Engine/Rendering/RenderGraph/RenderAccess.h>
#include <Engine/Rendering/RenderGraph/RenderNode.h>
#include <Engine/Rendering/RenderGraph/RenderResource.h>
#include <Runtime/Containers/Vector/List.h>
#include <Runtime/Containers/Memory/RefPtr.h>
#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/RHI/Buffer/Buffer.h>
#include <Runtime/RHI/Texture/Texture.h>
#include <Runtime/RHI/Texture/TextureView.h>

namespace Horizon::Engine
{
	class GraphicsContext;

	class ENGINE_API RenderGraph
	{
		struct RenderBarrier
		{
			Reflect::TypeHandle resource;
			RHI::ResourceState before = RHI::ResourceState::Undefined;
			RHI::ResourceState after = RHI::ResourceState::Undefined;
		};

		struct ResourceLifetime
		{
			usize firstAppear = u64_max;
			usize lastAppear = 0;
		};

	public:
		RenderGraph(Engine* pEngine);
		~RenderGraph();

		RenderGraph(const RenderGraph&) = delete;
		RenderGraph& operator=(const RenderGraph&) = delete;

		b8 OnInitialize();
		void OnExecute();
		void OnFinalize();

		void OnLibraryRegistered(const ReflectionLibrary& library);
		void OnLibraryUnregistered(const ReflectionLibrary& library);

		RenderResource* LookupResource(Reflect::TypeHandle resource, RenderResourceType type);

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

		b8 AcquireResources();
		void RecordRendering();

	private:
		Engine* m_engine = nullptr;
		ReflectionSystem* m_reflection = nullptr;
		JobSystem* m_job = nullptr;
		GraphicsContext* m_graphics = nullptr;

		List<RefPtr<RenderNode>> m_activeNodes;
		std::unordered_map<Reflect::TypeHandle, usize> m_nodeLookup;

		std::unordered_map<Reflect::TypeHandle, usize> m_creatorLookup; // Which indexed node called Creates<>()...
		List<List<usize>> m_adjacency;

		List<usize> m_order;

		std::unordered_map<Reflect::TypeHandle, ResourceLifetime> m_lifetimes;
		List<List<RenderBarrier>> m_nodeBarriers;

		std::unordered_map<Reflect::TypeHandle, RenderResource> m_resources; // My shitty pool
	};
}