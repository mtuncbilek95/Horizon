#include "FrameGraph.h"

#include <queue>

namespace Horizon
{
	void FrameGraph::SetGraphCache(std::unique_ptr<FrameGraphCache> cache)
	{
		m_cache = std::move(cache);
	}

	void FrameGraph::Import(const CompositePresentObject& presentObj)
	{
		m_backbufferHandle.idx = static_cast<u32>(m_resources.size());
		m_backbufferHandle.version = 0;

		ResourceEntry entry;
		entry.imported = true;
		entry.importedView = presentObj.imageView;

		m_resources.push_back(std::move(entry));
	}

	RenderTargetHandle FrameGraph::CreateResource(const std::string& name, const RenderTargetDesc& desc)
	{
		RenderTargetHandle handle;
		handle.idx = static_cast<u32>(m_resources.size());
		handle.version = 0;

		ResourceEntry entry;
		entry.desc = desc;
		entry.imported = false;

		m_resources.push_back(std::move(entry));
		return handle;
	}

	void FrameGraph::RegisterRead(u32 passIndex, RenderTargetHandle handle)
	{
		m_resources[handle.idx].readers.push_back(passIndex);
	}

	void FrameGraph::RegisterWrite(u32 passIndex, RenderTargetHandle handle)
	{
		m_resources[handle.idx].writer = passIndex;
	}

	void FrameGraph::Compile()
	{
		for (u32 i = 0; i < m_passes.size(); i++)
		{
			GraphBuilder builder(m_passNames[i], *this, i);
			m_passes[i]->Setup(builder);
		}

		u32 passCount = static_cast<u32>(m_passes.size());

		std::vector<std::vector<u32>> adjacency(passCount);
		std::vector<u32> inDegree(passCount, 0);

		for (auto& resource : m_resources)
		{
			if (resource.writer == u32_max)
				continue;

			for (u32 reader : resource.readers)
			{
				adjacency[resource.writer].push_back(reader);
				inDegree[reader]++;
			}
		}

		std::queue<u32> queue;
		for (u32 i = 0; i < passCount; i++)
		{
			if (inDegree[i] == 0)
				queue.push(i);
		}

		m_executionOrder.clear();
		while (!queue.empty())
		{
			u32 current = queue.front();
			queue.pop();
			m_executionOrder.push_back(current);

			for (u32 neighbor : adjacency[current])
			{
				inDegree[neighbor]--;
				if (inDegree[neighbor] == 0)
					queue.push(neighbor);
			}
		}

		assert(m_executionOrder.size() == passCount && "Cycle detected in FrameGraph");

		for (auto& resource : m_resources)
		{
			if (resource.imported)
				continue;

			resource.resource = &m_cache->GetOrCreate(resource.desc);
		}
	}

	void FrameGraph::Execute(GfxCommandBuffer* cmd)
	{
		PassResources resources(m_resources);

		for (u32 passIndex : m_executionOrder)
		{
			m_passes[passIndex]->Execute(cmd, resources);
		}
	}

	void FrameGraph::Reset()
	{
		for (auto& resource : m_resources)
		{
			if (!resource.imported && resource.resource != nullptr)
			{
				resource.resource->isInUse = false;
			}
		}

		m_resources.clear();
		m_executionOrder.clear();
		m_handleRegistry.clear();
		m_renderables.clear();
	}
}