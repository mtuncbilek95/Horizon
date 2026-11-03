#include "RenderGraph.h"

#include <Engine/Rendering/RenderGraph/RenderDependency.h>

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
		List<Reflect::Type*> nodeTypes = m_reflection->GetTypeByBase(Reflect::TypeOf<RenderNode>());
		for (auto* pType : nodeTypes)
		{
			RefPtr<RenderNode> pNode = MakeRef<RenderNode>(pType);
			if (!pNode)
			{
				Terminal::Error(StringHelper::GetTypeName(this), "{} may have missing v-table implementation. Check your virtual functions.", pType->GetName());
				continue;
			}

			pNode->m_engine = m_engine;

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

	void RenderGraph::OnExecute()
	{
		// Configure dependency list
		List<RenderDependency> dependencyList(m_activeNodes.GetCount());
		for (usize i = 0; i < m_activeNodes.GetCount(); i++)
			m_activeNodes[i]->Setup(dependencyList[i]);

		// Compile graph
		if (!CompileGraph(dependencyList))
			return;

		// Execute graph
	}

	void RenderGraph::OnFinalize()
	{
	}

	void RenderGraph::OnLibraryRegistered(const ReflectionLibrary& library)
	{
	}

	void RenderGraph::OnLibraryUnregistered(const ReflectionLibrary& library)
	{
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

		// TODO: Build lifetimes and barriers
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
				if (!m_creatorLookup.emplace(create.resource, i).second)
				{
					Terminal::Error(StringHelper::GetTypeName(this), "{} creates a texture that another node already creates.", nodeName);
					return false;
				}
			}

			// Resolve the buffers that has been created by the current pass
			for (const auto& create : depend.m_bufferCreates)
			{
				if (!m_creatorLookup.emplace(create.resource, i).second)
				{
					Terminal::Error(StringHelper::GetTypeName(this), "{} creates a buffer that another node already creates.", nodeName);
					return false;
				}
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
				if (m_creatorLookup.find(access.resource) == m_creatorLookup.end())
				{
					Terminal::Error(StringHelper::GetTypeName(this), "{} reads a resource that no node creates.", nodeName);
					return false;
				}
			}

			// Reading a resource that no node creates
			for (const auto& access : depend.m_writes)
			{
				if (m_creatorLookup.find(access.resource) == m_creatorLookup.end())
				{
					Terminal::Error(StringHelper::GetTypeName(this), "{} writes a resource that no node creates.", nodeName);
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
				usize creator = m_creatorLookup[access.resource];
				if (creator != i)
					AddEdge(adjacency, creator, i);
			}

			// Creator of each written resource runs before this node
			for (const auto& access : depend.m_writes)
			{
				usize creator = m_creatorLookup[access.resource];
				if (creator != i)
					AddEdge(adjacency, creator, i);
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
}