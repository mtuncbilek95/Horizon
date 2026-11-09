#include "RenderGraph.h"

#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Rendering/RenderGraph/RenderDependency.h>
#include <Runtime/RHI/Device/Device.h>
#include <Runtime/RHI/Command/TextureBarrier.h>
#include <Runtime/RHI/Command/CommandList.h>
#include <Runtime/RHI/Command/CommandPool.h>
#include <Runtime/RHI/Fence/Fence.h>
#include <Runtime/RTTR/Attributes/DynamicUsageAttribute.h>
#include <Runtime/RTTR/Attributes/IgnoreAttribute.h>

#include <queue>
#include <vector>

namespace Horizon::Engine
{
	RenderGraph::RenderGraph(Engine* pEngine) : m_engine(pEngine), m_reflection(pEngine->GetReflectionSystem()),
		m_job(pEngine->GetJobSystem())
	{
	}

	RenderGraph::~RenderGraph()
	{
	}

	b8 RenderGraph::OnInitialize()
	{
		m_graphics = m_engine->RequestContext<GraphicsContext>();
		if (!m_graphics)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "GraphicsContext could not be found. RenderGraph cannot initialize.");
			return false;
		}

		RHI::Device* pDevice = m_graphics->GetDevice();

		// Create a general graphics pool
		RHI::CommandPoolDesc poolDesc = {};
		poolDesc.type = RHI::QueueType::Graphics;

		m_pool = pDevice->CreateRHICommandPool(poolDesc);
		if (!m_pool)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Command pool could not be created.");
			return false;
		}

		// Create a single cmdList 
		RHI::CommandListDesc listDesc = {};
		listDesc.pPool = m_pool.Get();

		m_list = pDevice->CreateRHICommandList(listDesc);
		if (!m_list)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Command list could not be created.");
			return false;
		}

		// Create a fence to prevent fuckery
		m_fence = pDevice->CreateRHIFence(RHI::FenceDesc());
		if (!m_fence)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Fence could not be created.");
			return false;
		}

		// Gather all the RenderNodes 
		List<Reflect::Type*> nodeTypes = m_reflection->GetTypeByBase(Reflect::TypeOf<RenderNode>());
		for (auto* pType : nodeTypes)
		{
			if (pType->GetCustomAttribute<Reflect::IgnoreAttribute>())
				continue;

			if (pType->GetCustomAttribute<Reflect::DynamicUsageAttribute>())
				m_status[pType->GetTypeId()] = true;

			RefPtr<RenderNode> pNode = MakeRef<RenderNode>(pType);
			if (!pNode)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} may have missing v-table implementation. Check your virtual functions.", pType->GetName());
				continue;
			}

			pNode->m_engine = m_engine;
			pNode->m_graph = this;

			if (!pNode->OnInitialize())
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} has failed initializing. Check out OnInitialize to find the issue.", pType->GetName());
				continue;
			}

			m_nodeLookup[pType->GetTypeId()] = m_activeNodes.GetCount();
			m_activeNodes.PushBack(pNode);

			Terminal::Info(StringHelper::GetTypeName(this), "{} is ready to be used. Registered successfully.", pType->GetName());
		}

		return true;
	}

	void RenderGraph::OnExecute(const RenderSetupContext& setupContext, const RenderDrawContext& drawContext)
	{
		// Configure dependency list
		List<RenderDependency> dependencyList(m_activeNodes.GetCount());
		for (usize i = 0; i < m_activeNodes.GetCount(); i++)
			m_activeNodes[i]->Setup(setupContext, dependencyList[i]);

		// Compile graph
		if (!CompileGraph(dependencyList))
		{
			m_importedTextures.clear();
			return;
		}

		// Get missing resources
		if (!AcquireResources())
		{
			m_importedTextures.clear();
			return;
		}

		// Execute graph
		RecordRendering(drawContext);

		m_importedTextures.clear();
	}

	void RenderGraph::OnFinalize()
	{
		if (m_graphics)
			m_graphics->GetDevice()->WaitIdle();

		for (usize i = m_activeNodes.GetCount(); i > 0; i--)
			m_activeNodes[i - 1]->OnFinalize();

		m_resources.clear();
		m_importedTextures.clear();
		m_nodeBarriers.Clear();
		m_finalBarriers.Clear();
		m_lifetimes.clear();
		m_order.Clear();
		m_adjacency.Clear();
		m_creatorLookup.clear();

		m_nodeLookup.clear();
		m_activeNodes.Clear();
	}


	void RenderGraph::SetEnablePass(Reflect::TypeHandle handl, b8 enable)
	{
		auto it = m_status.find(handl);
		if (it == m_status.end())
			return;

		it->second = enable;
	}

	RHI::Texture* RenderGraph::FindTexture(Reflect::TypeHandle resource)
	{
		auto imported = m_importedTextures.find(resource);
		if (imported != m_importedTextures.end())
			return imported->second.pTexture;

		RenderResource* pResource = LookupResource(resource, RenderResourceType::Texture);
		if (!pResource)
			return nullptr;

		return pResource->texture.Get();
	}

	RHI::TextureView* RenderGraph::FindTextureView(Reflect::TypeHandle resource)
	{
		auto imported = m_importedTextures.find(resource);
		if (imported != m_importedTextures.end())
			return imported->second.pView;

		RenderResource* pResource = LookupResource(resource, RenderResourceType::Texture);
		if (!pResource)
			return nullptr;

		return pResource->texView.Get();
	}

	RHI::Buffer* RenderGraph::FindBuffer(Reflect::TypeHandle resource)
	{
		RenderResource* pResource = LookupResource(resource, RenderResourceType::Buffer);
		if (!pResource)
			return nullptr;

		return pResource->buffer.Get();
	}

	void RenderGraph::AddEdge(List<List<usize>>& adjacency, usize beforeIndex, usize afterIndex)
	{
		adjacency[beforeIndex].PushBack(afterIndex);
	}

	b8 RenderGraph::CompileGraph(List<RenderDependency>& dependencyList)
	{
		if (!ResolveResources(dependencyList))
			return false;

		m_adjacency = BuildExplicitEdges(dependencyList);

		List<usize> order;
		if (!TopologicalSort(order))
		{
			ReportCycle(order);
			return false;
		}

#if defined(HORIZON_DEBUG)
		// Check for ambiguities only in debug mode, because O(N^3).
		if (!ValidateAmbiguity(order, dependencyList))
			return false;
#endif

		BuildLifetimes(order, dependencyList);

		if (!BuildBarriers(order, dependencyList))
			return false;

		m_order = order;
		return true;
	}

	b8 RenderGraph::ResolveResources(List<RenderDependency>& dependencyList)
	{
		m_creatorLookup.clear();

		// Create buffers and textures 
		for (usize i = 0; i < dependencyList.GetCount(); i++)
		{
			RenderDependency& depend = dependencyList[i];
			const std::string& nodeName = m_reflection->GetType(m_activeNodes[i]->GetTypeId())->GetName();

			// Resolve the texture that has been created by the current pass
			for (const auto& create : depend.m_textureCreates)
			{
				Reflect::Type* pResourceType = m_reflection->GetType(create.resource);
				if (!pResourceType)
				{
					Terminal::Error(StringHelper::GetTypeName(this), "{} creates a texture whose tag is not reflected. Add HCLASS to the tag.", nodeName);
					return false;
				}

				if (m_importedTextures.find(create.resource) != m_importedTextures.end())
				{
					Terminal::Error(StringHelper::GetTypeName(this), "{} creates {} but it is imported into the graph.", nodeName, pResourceType->GetName());
					return false;
				}

				if (!m_creatorLookup.emplace(create.resource, i).second)
				{
					Terminal::Error(StringHelper::GetTypeName(this), "{} creates {} but another node already creates it.", nodeName, pResourceType->GetName());
					return false;
				}

				RenderResource& resource = m_resources[create.resource];
				resource.type = RenderResourceType::Texture;
				resource.texDesc = create.desc;
			}

			// Resolve the buffers that has been created by the current pass
			for (const auto& create : depend.m_bufferCreates)
			{
				Reflect::Type* pResourceType = m_reflection->GetType(create.resource);
				if (!pResourceType)
				{
					Terminal::Error(StringHelper::GetTypeName(this), "{} creates a buffer whose tag is not reflected. Add HCLASS to the tag.", nodeName);
					return false;
				}

				if (!m_creatorLookup.emplace(create.resource, i).second)
				{
					Terminal::Error(StringHelper::GetTypeName(this), "{} creates {} but another node already creates it.", nodeName, pResourceType->GetName());
					return false;
				}

				RenderResource& resource = m_resources[create.resource];
				resource.type = RenderResourceType::Buffer;
				resource.bufDesc = create.desc;
			}
		}

		// Check if there is a invalid usage
		for (usize i = 0; i < dependencyList.GetCount(); i++)
		{
			RenderDependency& depend = dependencyList[i];
			const std::string& nodeName = m_reflection->GetType(m_activeNodes[i]->GetTypeId())->GetName();

			// Reading a resource that no node creates
			for (const auto& access : depend.m_reads)
			{
				if (m_creatorLookup.find(access.resource) == m_creatorLookup.end() && m_importedTextures.find(access.resource) == m_importedTextures.end())
				{
					Terminal::Error(StringHelper::GetTypeName(this), "{} reads a resource that is neither created nor imported.", nodeName);
					return false;
				}
			}

			// Reading a resource that no node creates
			for (const auto& access : depend.m_writes)
			{
				if (m_creatorLookup.find(access.resource) == m_creatorLookup.end() && m_importedTextures.find(access.resource) == m_importedTextures.end())
				{
					Terminal::Error(StringHelper::GetTypeName(this), "{} writes a resource that is neither created nor imported.", nodeName);
					return false;
				}
			}
		}

		return true;
	}

	List<List<usize>> RenderGraph::BuildExplicitEdges(List<RenderDependency>& dependencyList)
	{
		List<List<usize>> adjacency(m_activeNodes.GetCount());
		for (usize i = 0; i < m_activeNodes.GetCount(); i++)
		{
			RenderDependency& depend = dependencyList[i];

			// Regular edge topo sort
			for (const auto& handle : depend.m_runsBefore)
			{
				auto it = m_nodeLookup.find(handle);
				if (it == m_nodeLookup.end())
				{
					Terminal::Warn(StringHelper::GetTypeName(this), "RunsBefore target is not an active node. Edge skipped.");
					continue;
				}

				AddEdge(adjacency, i, it->second);
			}

			// Regular edge topo sort
			for (const auto& handle : depend.m_runsAfter)
			{
				auto it = m_nodeLookup.find(handle);
				if (it == m_nodeLookup.end())
				{
					Terminal::Warn(StringHelper::GetTypeName(this), "RunsAfter target is not an active node. Edge skipped.");
					continue;
				}

				AddEdge(adjacency, it->second, i);
			}

			// Creator of each read resource runs before this node
			for (const auto& access : depend.m_reads)
			{
				auto it = m_creatorLookup.find(access.resource);
				if (it == m_creatorLookup.end())
					continue;

				if (it->second != i)
					AddEdge(adjacency, it->second, i);
			}

			// Creator of each written resource runs before this node
			for (const auto& access : depend.m_writes)
			{
				auto it = m_creatorLookup.find(access.resource);
				if (it == m_creatorLookup.end())
					continue;

				if (it->second != i)
					AddEdge(adjacency, it->second, i);
			}
		}

		return adjacency;
	}

	b8 RenderGraph::TopologicalSort(List<usize>& order)
	{
		List<usize> indegrees(m_adjacency.GetCount(), 0);
		std::priority_queue<usize, std::vector<usize>, std::greater<>> systemQ;

		for (usize i = 0; i < m_adjacency.GetCount(); i++)
		{
			for (usize next : m_adjacency[i])
				indegrees[next]++;
		}

		// Get the first independent ones
		for (usize i = 0; i < m_adjacency.GetCount(); i++)
		{
			if (indegrees[i] == 0)
				systemQ.push(i);
		}

		// Resolve the rest
		while (!systemQ.empty())
		{
			usize idx = systemQ.top();
			systemQ.pop();

			order.PushBack(idx);

			for (usize next : m_adjacency[idx])
			{
				indegrees[next]--;

				if (indegrees[next] == 0)
					systemQ.push(next);
			}
		}

		return m_adjacency.GetCount() == order.GetCount();
	}

	void RenderGraph::ReportCycle(List<usize>& order)
	{
		// TODO: Make a better report later
		Terminal::Error(StringHelper::GetTypeName(this), "RenderGraph Cycle! ! ! ! !");
	}

	b8 RenderGraph::ValidateAmbiguity(List<usize>& order, List<RenderDependency>& dependencyList)
	{
		struct Accessor
		{
			usize node = 0;
			b8 writes = false;
		};

		usize nodeCount = m_activeNodes.GetCount();

		// List of "where can I arrive in the end?"
		List<List<b8>> reachable(nodeCount, List<b8>(nodeCount, false));

		// Start from the end
		for (usize idx = order.GetCount(); idx > 0; idx--)
		{
			// Scan the very end
			usize node = order[idx - 1];
			for (usize next : m_adjacency[node])
			{
				reachable[node][next] = true; // Mark
				for (usize i = 0; i < nodeCount; i++)
				{
					if (reachable[next][i])
						reachable[node][i] = true; // Then mark for this as well
				}
			}
		}

		// Get the reads and writes 
		std::unordered_map<Reflect::TypeHandle, List<Accessor>> accessorsByResource;
		for (usize i = 0; i < dependencyList.GetCount(); i++)
		{
			for (const auto& access : dependencyList[i].m_reads)
				accessorsByResource[access.resource].PushBack({ i, false });

			for (const auto& access : dependencyList[i].m_writes)
				accessorsByResource[access.resource].PushBack({ i, true });
		}

		// Check if a resource is being accessed via 2 nodes at the same time.
		for (const auto& pair : accessorsByResource)
		{
			const List<Accessor>& accessors = pair.second;
			for (usize a = 0; a < accessors.GetCount(); a++)
			{
				for (usize b = a + 1; b < accessors.GetCount(); b++)
				{
					const Accessor& lhs = accessors[a];
					const Accessor& rhs = accessors[b];

					if (lhs.node == rhs.node || (!lhs.writes && !rhs.writes))
						continue;

					if (reachable[lhs.node][rhs.node] || reachable[rhs.node][lhs.node])
						continue;

					Terminal::Error(StringHelper::GetTypeName(this), "{} and {} access the same resource without an order. Add RunsBefore or RunsAfter.",
						m_reflection->GetType(m_activeNodes[lhs.node]->GetTypeId())->GetName(), m_reflection->GetType(m_activeNodes[rhs.node]->GetTypeId())->GetName());
					return false;
				}
			}
		}

		return true;
	}

	void RenderGraph::BuildLifetimes(const List<usize>& order, List<RenderDependency>& dependencyList)
	{
		m_lifetimes.clear();

		for (usize i = 0; i < order.GetCount(); i++)
		{
			RenderDependency& depend = dependencyList[order[i]];

			for (const auto& access : depend.m_reads)
			{
				ResourceLifetime& lifetime = m_lifetimes[access.resource];
				if (lifetime.firstAppear == u64_max)
					lifetime.firstAppear = i;

				lifetime.lastAppear = i;
			}

			for (const auto& access : depend.m_writes)
			{
				ResourceLifetime& lifetime = m_lifetimes[access.resource];
				if (lifetime.firstAppear == u64_max)
					lifetime.firstAppear = i;

				lifetime.lastAppear = i;
			}
		}
	}

	b8 RenderGraph::BuildBarriers(const List<usize>& order, List<RenderDependency>& dependencyList)
	{
		struct TrackedState
		{
			RHI::ResourceState state = RHI::ResourceState::Undefined;
			b8 written = false;
		};

		struct NodeAccess
		{
			RHI::ResourceState state = RHI::ResourceState::Undefined;
			b8 writes = false;
		};

		m_nodeBarriers = List<List<RenderBarrier>>(m_activeNodes.GetCount());

		std::unordered_map<Reflect::TypeHandle, TrackedState> trackedStates;
		std::unordered_map<Reflect::TypeHandle, NodeAccess> nodeAccesses;

		for (const auto& pair : m_importedTextures)
			trackedStates[pair.first] = { pair.second.initialState, false };

		// Start running order list
		for (usize node : order)
		{
			nodeAccesses.clear();

			RenderDependency& depend = dependencyList[node];
			for (const auto& access : depend.m_reads)
			{
				auto it = nodeAccesses.find(access.resource);
				if (it == nodeAccesses.end()) // If it's a new node, add the state
					nodeAccesses[access.resource] = { access.state, false };
				else
					it->second.state |= access.state; // Combine with previous state
			}

			for (const auto& access : depend.m_writes)
			{
				auto it = nodeAccesses.find(access.resource);
				if (it != nodeAccesses.end() && it->second.state != access.state)
				{
					Terminal::Error(StringHelper::GetTypeName(this), "{} reads and writes the same resource in different states.",
						m_reflection->GetType(m_activeNodes[node]->GetTypeId())->GetName());
					return false;
				}

				nodeAccesses[access.resource] = { access.state, true };
			}

			for (const auto& pair : nodeAccesses)
			{
				const NodeAccess& access = pair.second;
				TrackedState& tracked = trackedStates[pair.first];

				b8 transition = tracked.state != access.state;
				b8 uavHazard = access.state == RHI::ResourceState::UnorderedAccess && (tracked.written || access.writes);

				if (transition || uavHazard)
					m_nodeBarriers[node].PushBack({ pair.first, tracked.state, access.state });

				tracked.state = access.state;
				tracked.written = access.writes;
			}
		}

		m_finalBarriers.Clear();
		for (const auto& pair : m_importedTextures)
		{
			TrackedState& tracked = trackedStates[pair.first];
			if (tracked.state != pair.second.finalState)
				m_finalBarriers.PushBack({ pair.first, tracked.state, pair.second.finalState });
		}

		return true;
	}

	b8 RenderGraph::AcquireResources()
	{
		RHI::Device* pDevice = m_graphics->GetDevice();

		for (auto& pair : m_resources)
		{
			RenderResource& resource = pair.second;

			// Process buffer
			if (resource.type == RenderResourceType::Buffer)
			{
				if (resource.buffer)
					continue;

				resource.buffer = pDevice->CreateRHIBuffer(resource.bufDesc);
				if (!resource.buffer)
				{
					Terminal::Error(StringHelper::GetTypeName(this), "{} buffer could not be created.", m_reflection->GetType(pair.first)->GetName());
					return false;
				}

				continue;
			}

			if (resource.texture)
				continue;

			// Continue processing texture
			resource.texture = pDevice->CreateRHITexture(resource.texDesc);
			if (!resource.texture)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} texture could not be created.", m_reflection->GetType(pair.first)->GetName());
				return false;
			}

			RHI::TextureViewDesc viewDesc = {};
			viewDesc.pTexture = resource.texture.Get();
			viewDesc.format = resource.texDesc.format;

			b8 isArray = resource.texDesc.arrayLayers > 1;
			switch (resource.texDesc.type)
			{
			case RHI::TextureType::Texture1D:
				viewDesc.type = isArray ? RHI::TextureViewType::Texture1DArray : RHI::TextureViewType::Texture1D;
				break;
			case RHI::TextureType::Texture2D:
				viewDesc.type = isArray ? RHI::TextureViewType::Texture2DArray : RHI::TextureViewType::Texture2D;
				break;
			case RHI::TextureType::Texture3D:
				viewDesc.type = RHI::TextureViewType::Texture3D;
				break;
			case RHI::TextureType::TextureCube:
				viewDesc.type = resource.texDesc.arrayLayers > 6 ? RHI::TextureViewType::TextureCubeArray : RHI::TextureViewType::TextureCube;
				break;
			}

			resource.texView = pDevice->CreateRHITextureView(viewDesc);
			if (!resource.texView)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} texture view could not be created.", m_reflection->GetType(pair.first)->GetName());
				return false;
			}
		}

		return true;
	}

	RenderResource* RenderGraph::LookupResource(Reflect::TypeHandle resource, RenderResourceType type)
	{
		auto it = m_resources.find(resource);
		if (it == m_resources.end())
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Requested resource is not created by any node.");
			return nullptr;
		}

		if (it->second.type != type)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "{} is requested with the wrong resource type.", m_reflection->GetType(resource)->GetName());
			return nullptr;
		}

		return &it->second;
	}

	void RenderGraph::RecordRendering(const RenderDrawContext& drawContext)
	{
		auto* pDevice = m_graphics->GetDevice();

		m_fence->WaitCPU(m_fenceValue);
		m_pool->Reset();

		// Start command-list 
		m_list->Start();
		m_list->BindDescriptorHeaps(m_graphics->GetResourceHeap(), m_graphics->GetSamplerHeap());

		for (usize node : m_order)
		{
			RecordBarriers(m_nodeBarriers[node]);

			m_activeNodes[node]->m_cmdList = m_list.Get();
			if (!m_status[m_activeNodes[node]->GetTypeId()])
				continue;
			m_activeNodes[node]->Render(drawContext);
			m_activeNodes[node]->m_cmdList = nullptr;
		}

		RecordBarriers(m_finalBarriers);

		m_list->End();
		m_fenceValue++;

		pDevice->Submit(RHI::QueueType::Graphics, { m_list.Get() });
		m_fenceValue = pDevice->Signal(RHI::QueueType::Graphics, m_fence.Get());
	}

	void RenderGraph::RecordBarriers(const List<RenderBarrier>& barriers)
	{
		for (const RenderBarrier& barrier : barriers)
		{
			auto imported = m_importedTextures.find(barrier.resource);
			if (imported != m_importedTextures.end())
			{
				RHI::TextureBarrier texBarrier = {};
				texBarrier.pTexture = imported->second.pTexture;
				texBarrier.before = barrier.before;
				texBarrier.after = barrier.after;

				m_list->Barrier({ texBarrier });
				continue;
			}

			RenderResource& resource = m_resources[barrier.resource];

			if (resource.type == RenderResourceType::Texture)
			{
				RHI::TextureBarrier texBarrier = {};
				texBarrier.pTexture = resource.texture.Get();
				texBarrier.before = barrier.before;
				texBarrier.after = barrier.after;

				m_list->Barrier({ texBarrier });
			}
			else
			{
				RHI::BufferBarrier bufBarrier = {};
				bufBarrier.pBuffer = resource.buffer.Get();
				bufBarrier.before = barrier.before;
				bufBarrier.after = barrier.after;

				m_list->Barrier({ bufBarrier });
			}
		}
	}
}