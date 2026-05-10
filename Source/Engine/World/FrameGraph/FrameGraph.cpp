#include "FrameGraph.h"

#include <queue>

namespace Horizon
{
	void FrameGraph::SetGraphCache(std::unique_ptr<FrameGraphCache> cache)
	{
		m_cache = std::move(cache);
	}

	RenderTargetHandle FrameGraph::Import(const CompositePresentObject& presentObj)
	{
		RenderTargetHandle handle;
		handle.idx = static_cast<u32>(m_resources.size());
		handle.version = 0;

		RenderTargetEntry imported;
		imported.view = std::shared_ptr<GfxImageView>(presentObj.imageView, [](GfxImageView*) {});
		imported.isInUse = true;
		m_importedResources.push_back(std::move(imported));

		ResourceEntry entry;
		entry.imported = true;
		entry.resource = &m_importedResources.back();

		m_resources.push_back(std::move(entry));
		return handle;
	}

	void FrameGraph::AddPass(const std::string& name, std::function<void(PassBuilder&)> setup,
		std::function<void(GfxCommandBuffer*, const PassResources&)> execute)
	{
		u32 passIndex = static_cast<u32>(m_passes.size());

		PassEntry pass;
		pass.name = name;
		pass.execute = std::move(execute);
		m_passes.push_back(std::move(pass));

		PassBuilder builder(*this, passIndex);
		setup(builder);
	}

	RenderTargetHandle FrameGraph::CreateResource(const RenderTargetDesc& desc)
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
		m_passes[passIndex].reads.push_back(handle);
	}

	void FrameGraph::RegisterWrite(u32 passIndex, RenderTargetHandle handle)
	{
		m_resources[handle.idx].writer = passIndex;
		m_passes[passIndex].writes.push_back(handle);
	}

	void FrameGraph::Compile()
	{
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
			m_passes[passIndex].execute(cmd, resources);
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
		m_passes.clear();
		m_executionOrder.clear();
		m_importedResources.clear();
	}
}