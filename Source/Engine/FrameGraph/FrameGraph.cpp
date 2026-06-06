#include "FrameGraph.h"

#include <Engine/Log/Log.h>
#include <Engine/Graphics/RHI/GfxQueue.h>

#include <unordered_map>

namespace Horizon
{
	static b8 TextureDescMatches(const GfxTextureDesc& descA, const GfxTextureDesc& descB)
	{
		return descA.width == descB.width && descA.height == descB.height && descA.depth == descB.depth &&
			descA.mipLevels == descB.mipLevels && descA.format == descB.format &&
			descA.usage == descB.usage && descA.type == descB.type;
	}

	static b8 BufferDescMatches(const GfxBufferDesc& descA, const GfxBufferDesc& descB)
	{
		return descA.size == descB.size && descA.stride == descB.stride &&
			descA.usage == descB.usage && descA.memory == descB.memory;
	}

	FrameGraph::FrameGraph()
	{
		m_passes.reserve(MaxRenderPass);
		m_textureResources.reserve(MaxTextureResources);
		m_bufferResources.reserve(MaxBufferResources);
	}

	FrameGraph::~FrameGraph()
	{
		Shutdown();
	}

	FGTextureHandle FrameGraph::ImportTexture(GfxTextureHandle texture, GfxResourceState currentState,
		std::string_view name, u32 width, u32 height, u32 shaderViewIndex)
	{
		FrameGraphTextureResource resource = {};
		resource.name = name;
		resource.physical = texture;
		resource.initialState = currentState;
		resource.imported = true;
		resource.version = 0;
		resource.shaderViewIndex = shaderViewIndex;
		resource.desc.width = width;
		resource.desc.height = height;

		u32 resourceIndex = u32(m_textureResources.size());
		m_textureResources.push_back(resource);

		return FGTextureHandle::make(resourceIndex, 0);
	}

	FGBufferHandle FrameGraph::ImportBuffer(GfxBufferHandle buffer, GfxResourceState currentState,
		std::string_view name, u32 shaderViewIndex)
	{
		FrameGraphBufferResource resource = {};
		resource.name = name;
		resource.physical = buffer;
		resource.initialState = currentState;
		resource.imported = true;
		resource.version = 0;
		resource.shaderViewIndex = shaderViewIndex;

		u32 resourceIndex = (u32)m_bufferResources.size();
		m_bufferResources.push_back(resource);
		return FGBufferHandle::make(resourceIndex, 0);
	}

	u32 FrameGraph::RegisterTransientTexture(const GfxTextureDesc& desc, std::string_view name)
	{
		FrameGraphTextureResource resource = {};
		resource.name = name;
		resource.desc = desc;
		resource.imported = false;
		resource.version = 0;

		u32 resourceIndex = u32(m_textureResources.size());
		m_textureResources.push_back(resource);

		return resourceIndex;
	}

	u32 FrameGraph::RegisterTransientBuffer(const GfxBufferDesc& desc, std::string_view name)
	{
		FrameGraphBufferResource resource = {};
		resource.name = name;
		resource.desc = desc;
		resource.imported = false;
		resource.version = 0;

		u32 resourceIndex = (u32)m_bufferResources.size();
		m_bufferResources.push_back(resource);

		return resourceIndex;
	}

	u32 FrameGraph::NextTextureVersion(u32 resourceIndex)
	{
		return ++m_textureResources[resourceIndex].version;
	}

	u32 FrameGraph::NextBufferVersion(u32 resourceIndex)
	{
		return ++m_bufferResources[resourceIndex].version;
	}

	void FrameGraph::RegisterPass(std::unique_ptr<FrameGraphPass> pass)
	{
		m_passes.push_back(std::move(pass));
	}

	void FrameGraph::CullPasses()
	{
		struct CompiledVersion
		{
			FrameGraphResourceType type;
			u32 resourceIndex;
			u32 version;
			i32 producer;
			u32 readerCount;
		};

		std::vector<CompiledVersion> nodes;
		std::unordered_map<u64, u32> nodeLookup;

		auto keyOf = [](const FrameGraphAccess& access) -> u64
			{
				u64 typeBit = (access.type == FrameGraphResourceType::Buffer) ? 1ull : 0ull;
				return (typeBit << 63) | (u64(access.resourceIndex) << 32) | u64(access.version);
			};

		auto getOrAddNode = [&](const FrameGraphAccess& access) -> u32
			{
				u64 key = keyOf(access);
				auto found = nodeLookup.find(key);
				if (found != nodeLookup.end())
					return found->second;

				u32 nodeId = u32(nodes.size());
				nodes.push_back({ access.type, access.resourceIndex, access.version, -1, 0 });
				nodeLookup.emplace(key, nodeId);

				return nodeId;
			};

		std::vector<u32> outputRef(m_passes.size(), 0);

		// Producers (writes) + output count per pass
		for (u32 passIndex = 0; passIndex < u32(m_passes.size()); passIndex++)
		{
			FrameGraphPass& pass = *m_passes[passIndex];
			pass.m_culled = false;

			for (const FrameGraphAccess& write : pass.m_writes)
			{
				u32 nodeId = getOrAddNode(write);
				nodes[nodeId].producer = i32(passIndex);
			}
			outputRef[passIndex] = u32(pass.m_writes.size());
		}

		// Reader counts
		for (u32 passIndex = 0; passIndex < (u32)m_passes.size(); passIndex++)
		{
			FrameGraphPass& pass = *m_passes[passIndex];
			for (const FrameGraphAccess& read : pass.m_reads)
			{
				u32 nodeId = getOrAddNode(read);
				nodes[nodeId].readerCount++;
			}
		}

		// Filter the fuck out
		std::vector<u32> stack;
		for (u32 nodeId = 0; nodeId < u32(nodes.size()); nodeId++)
		{
			if (nodes[nodeId].readerCount == 0)
				stack.push_back(nodeId);
		}

		while (!stack.empty())
		{
			u32 nodeId = stack.back();
			stack.pop_back();

			i32 producer = nodes[nodeId].producer;

			if (producer < 0)
				continue;

			if (m_passes[producer]->m_hasSideEffect)
				continue;

			if (--outputRef[producer] != 0)
				continue;

			for (const FrameGraphAccess& read : m_passes[producer]->m_reads)
			{
				u32 readNode = getOrAddNode(read);

				if (--nodes[readNode].readerCount == 0)
					stack.push_back(readNode);
			}
		}

		// Pin the survived ones
		for (u32 passIndex = 0; passIndex < (u32)m_passes.size(); passIndex++)
		{
			FrameGraphPass& pass = *m_passes[passIndex];
			pass.m_culled = (outputRef[passIndex] == 0) && !pass.m_hasSideEffect;
		}
	}

	void FrameGraph::ComputeLifetimes()
	{
		for (u32 passIndex = 0; passIndex < u32(m_passes.size()); passIndex++)
		{
			FrameGraphPass& pass = *m_passes[passIndex];
			if (pass.m_culled)
				continue;

			auto touch = [&](const FrameGraphAccess& access)
				{
					if (access.type == FrameGraphResourceType::Texture)
					{
						FrameGraphTextureResource& resource = m_textureResources[access.resourceIndex];

						if (resource.firstPass < 0)
							resource.firstPass = i32(passIndex);

						resource.lastPass = i32(passIndex);
						resource.refCount++;
					}
					else
					{
						FrameGraphBufferResource& resource = m_bufferResources[access.resourceIndex];

						if (resource.firstPass < 0)
							resource.firstPass = i32(passIndex);

						resource.lastPass = i32(passIndex);
						resource.refCount++;
					}
				};

			for (const FrameGraphAccess& write : pass.m_writes)
				touch(write);

			for (const FrameGraphAccess& read : pass.m_reads)
				touch(read);
		}
	}

	void FrameGraph::AllocateResources()
	{
		for (FrameGraphTexturePoolEntry& entry : m_texturePool)
			entry.inUse = false;

		for (FrameGraphBufferPoolEntry& entry : m_bufferPool)
			entry.inUse = false;

		for (FrameGraphTextureResource& resource : m_textureResources)
		{
			if (resource.imported)
				continue;
			if (resource.firstPass < 0)
				continue;

			u32 entryIndex = AcquireTextureFromPool(resource.desc);
			FrameGraphTexturePoolEntry& entry = m_texturePool[entryIndex];
			resource.physical = entry.texture;
			resource.shaderViewIndex = entry.shaderViewIndex;
			resource.accessViewIndex = entry.accessViewIndex;
			resource.poolEntry = (i32)entryIndex;
		}

		for (FrameGraphBufferResource& resource : m_bufferResources)
		{
			if (resource.imported)
				continue;
			if (resource.firstPass < 0)
				continue;

			u32 entryIndex = AcquireBufferFromPool(resource.desc);
			FrameGraphBufferPoolEntry& entry = m_bufferPool[entryIndex];
			resource.physical = entry.buffer;
			resource.shaderViewIndex = entry.shaderViewIndex;
			resource.accessViewIndex = entry.accessViewIndex;
			resource.poolEntry = (i32)entryIndex;
		}
	}

	u32 FrameGraph::AcquireTextureFromPool(const GfxTextureDesc& desc)
	{
		for (u32 entryIndex = 0; entryIndex < (u32)m_texturePool.size(); entryIndex++)
		{
			FrameGraphTexturePoolEntry& entry = m_texturePool[entryIndex];
			if (!entry.inUse && TextureDescMatches(entry.desc, desc))
			{
				entry.inUse = true;
				return entryIndex;
			}
		}

		FrameGraphTexturePoolEntry entry = {};
		entry.desc = desc;
		entry.texture = GfxDevice::CreateTexture(desc);

		if (has(desc.usage, GfxTextureUsage::Sampled))
			entry.shaderViewIndex = GfxDevice::BindlessShaderView(entry.texture);

		if (has(desc.usage, GfxTextureUsage::Storage))
			entry.accessViewIndex = GfxDevice::BindlessAccessView(entry.texture);
		entry.inUse = true;

		m_texturePool.push_back(entry);
		return u32(m_texturePool.size()) - 1;
	}

	u32 FrameGraph::AcquireBufferFromPool(const GfxBufferDesc& desc)
	{
		for (u32 entryIndex = 0; entryIndex < (u32)m_bufferPool.size(); entryIndex++)
		{
			FrameGraphBufferPoolEntry& entry = m_bufferPool[entryIndex];
			if (!entry.inUse && BufferDescMatches(entry.desc, desc))
			{
				entry.inUse = true;
				return entryIndex;
			}
		}

		FrameGraphBufferPoolEntry entry = {};
		entry.desc = desc;
		entry.buffer = GfxDevice::CreateBuffer(desc);
		entry.shaderViewIndex = GfxDevice::BindlessShaderView(entry.buffer);

		if (has(desc.usage, GfxBufferUsage::Storage))
			entry.shaderViewIndex = GfxDevice::BindlessAccessView(entry.buffer);
		entry.inUse = true;

		m_bufferPool.push_back(entry);
		return u32(m_bufferPool.size()) - 1;
	}

	void FrameGraph::BuildBarriers()
	{
		m_finalTextureBarriers.clear();
		m_finalBufferBarriers.clear();

		for (FrameGraphTextureResource& resource : m_textureResources)
		{
			if (resource.firstPass < 0)
				continue;

			resource.currentState = resource.imported ? resource.initialState : m_texturePool[resource.poolEntry].state;
		}

		for (FrameGraphBufferResource& resource : m_bufferResources)
		{
			if (resource.firstPass < 0)
				continue;

			resource.currentState = resource.imported ? resource.initialState : m_bufferPool[resource.poolEntry].state;
		}

		for (u32 passIndex = 0; passIndex < u32(m_passes.size()); passIndex++)
		{
			FrameGraphPass& pass = *m_passes[passIndex];

			if (pass.m_culled)
				continue;

			auto transition = [&](const FrameGraphAccess& access)
				{
					if (access.type == FrameGraphResourceType::Texture)
					{
						FrameGraphTextureResource& resource = m_textureResources[access.resourceIndex];
						if (resource.currentState == access.state)
							return;

						pass.m_textureBarriers.push_back({ resource.physical, resource.currentState, access.state });
						resource.currentState = access.state;
					}
					else
					{
						FrameGraphBufferResource& resource = m_bufferResources[access.resourceIndex];
						if (resource.currentState == access.state)
							return;

						pass.m_bufferBarriers.push_back({ resource.physical, resource.currentState, access.state });
						resource.currentState = access.state;
					}
				};

			for (const FrameGraphAccess& write : pass.m_writes)
				transition(write);
			for (const FrameGraphAccess& read : pass.m_reads)
				transition(read);
		}

		for (FrameGraphTextureResource& resource : m_textureResources)
		{
			if (resource.firstPass < 0)
				continue;

			if (resource.imported)
			{
				if (resource.currentState != resource.initialState)
					m_finalTextureBarriers.push_back({ resource.physical, resource.currentState, resource.initialState });
			}
			else
				m_texturePool[resource.poolEntry].state = resource.currentState;
		}

		for (FrameGraphBufferResource& resource : m_bufferResources)
		{
			if (resource.firstPass < 0)
				continue;

			if (resource.imported)
			{
				if (resource.currentState != resource.initialState)
					m_finalBufferBarriers.push_back({ resource.physical, resource.currentState, resource.initialState });
			}
			else
				m_bufferPool[resource.poolEntry].state = resource.currentState;
		}
	}

	GfxRenderPassBegin FrameGraph::BuildRenderPass(const FrameGraphPass& pass) const
	{
		GfxRenderPassBegin renderPass = {};
		renderPass.colorCount = pass.m_colorCount;

		u32 width = 0;
		u32 height = 0;

		for (u32 slot = 0; slot < pass.m_colorCount; slot++)
		{
			const FrameGraphColorAttachment& attachment = pass.m_colorAttachments[slot];
			const FrameGraphTextureResource& resource = m_textureResources[attachment.resourceIndex];

			GfxColorAttachment& color = renderPass.colors[slot];
			color.target = resource.physical;
			color.load = attachment.load;
			color.store = attachment.store;
			color.clear = resource.desc.clearColor;

			width = resource.desc.width;
			height = resource.desc.height;
		}

		if (pass.m_depthAttachment.enabled)
		{
			const FrameGraphDepthAttachment& attachment = pass.m_depthAttachment;
			const FrameGraphTextureResource& resource = m_textureResources[attachment.resourceIndex];

			renderPass.depth.target = resource.physical;
			renderPass.depth.load = attachment.load;
			renderPass.depth.store = attachment.store;
			renderPass.depth.clearDepth = resource.desc.clearDepth;
			renderPass.hasDepth = true;

			width = resource.desc.width;
			height = resource.desc.height;
		}

		renderPass.width = width;
		renderPass.height = height;

		return renderPass;
	}

	void FrameGraph::Reset()
	{
		m_passes.clear();
		m_textureResources.clear();
		m_bufferResources.clear();
	}

	void FrameGraph::Shutdown()
	{
		for (FrameGraphTexturePoolEntry& entry : m_texturePool)
			GfxDevice::DestroyTexture(entry.texture);
		for (FrameGraphBufferPoolEntry& entry : m_bufferPool)
			GfxDevice::DestroyBuffer(entry.buffer);

		m_texturePool.clear();
		m_bufferPool.clear();
	}

	void FrameGraph::Compile()
	{
		CullPasses();
		ComputeLifetimes();
		AllocateResources();
		BuildBarriers();
	}

	void FrameGraph::Execute()
	{
		FrameGraphResources resources(*this);
		GfxCommandList* cmdList = GfxDevice::AcquireCmd(GfxQueueType::Graphics, 0);

		for (u32 passIndex = 0; passIndex < (u32)m_passes.size(); passIndex++)
		{
			FrameGraphPass& pass = *m_passes[passIndex];
			if (pass.m_culled)
				continue;

			if (!pass.m_textureBarriers.empty())
				GfxCmdList::Barrier(cmdList, { pass.m_textureBarriers.data(), pass.m_textureBarriers.size() });
			if (!pass.m_bufferBarriers.empty())
				GfxCmdList::Barrier(cmdList, { pass.m_bufferBarriers.data(), pass.m_bufferBarriers.size() });

			GfxCmdList::BeginMarker(cmdList, pass.m_name, { 80, 160, 220 });

			if (pass.m_raster)
			{
				GfxRenderPassBegin renderPass = BuildRenderPass(pass);

				GfxCmdList::BeginRenderPass(cmdList, renderPass);
				pass.Execute(resources, cmdList);
				GfxCmdList::EndRenderPass(cmdList);
			}
			else
				pass.Execute(resources, cmdList);

			GfxCmdList::EndMarker(cmdList);
		}

		if (!m_finalTextureBarriers.empty())
			GfxCmdList::Barrier(cmdList, { m_finalTextureBarriers.data(), m_finalTextureBarriers.size() });
		if (!m_finalBufferBarriers.empty())
			GfxCmdList::Barrier(cmdList, { m_finalBufferBarriers.data(), m_finalBufferBarriers.size() });

		GfxQueue::Submit(GfxQueueType::Graphics, { { &cmdList, 1 }, {}, {} });
	}
}