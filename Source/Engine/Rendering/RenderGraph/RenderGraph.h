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
#include <Runtime/RHI/Queue/QueueType.h>

namespace Horizon::RHI
{
	class Fence;
	class CommandPool;
	class CommandList;
}

namespace Horizon::Engine
{
	class GraphicsContext;

	class RenderGraph
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

		struct ImportedTexture
		{
			RHI::Texture* pTexture = nullptr;
			RHI::TextureView* pView = nullptr;
			RHI::ResourceState initialState = RHI::ResourceState::Undefined;
			RHI::ResourceState finalState = RHI::ResourceState::Undefined;
		};

	public:
		RenderGraph(Engine* pEngine);
		~RenderGraph();

		RenderGraph(const RenderGraph&) = delete;
		RenderGraph& operator=(const RenderGraph&) = delete;

		b8 OnInitialize();
		void OnExecute(const RenderSetupContext& setupContext, const RenderDrawContext& drawContext);
		void OnFinalize();

		template<typename T>
		void ImportTexture(RHI::Texture* pTexture, RHI::TextureView* pView, RHI::ResourceState initialState, RHI::ResourceState finalState)
		{
			m_importedTextures[Reflect::TypeOf<T>()] = { pTexture, pView, initialState, finalState };
		}

		void SetEnablePass(Reflect::TypeHandle handl, b8 enable);

		RHI::Texture* FindTexture(Reflect::TypeHandle resource);
		RHI::TextureView* FindTextureView(Reflect::TypeHandle resource);
		RHI::Buffer* FindBuffer(Reflect::TypeHandle resource);

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
		void RecordRendering(const RenderDrawContext& drawContext);
		void RecordBarriers(const List<RenderBarrier>& barriers);

		RenderResource* LookupResource(Reflect::TypeHandle resource, RenderResourceType type);

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
		List<RenderBarrier> m_finalBarriers;

		std::unordered_map<Reflect::TypeHandle, RenderResource> m_resources; // My shitty pool
		std::unordered_map<Reflect::TypeHandle, ImportedTexture> m_importedTextures;

		UniquePtr<RHI::CommandPool> m_pool = nullptr;
		UniquePtr<RHI::CommandList> m_list = nullptr;
		UniquePtr<RHI::Fence> m_fence = nullptr;
		u64 m_fenceValue = 0;

		std::unordered_map<Reflect::TypeHandle, b8> m_status;
	};
}