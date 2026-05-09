#include <Engine/FrameGraph/FrameGraph.h>

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Image/GfxImage.h>
#include <Runtime/Graphics/RHI/Image/GfxImageView.h>
#include <Runtime/Graphics/RHI/Buffer/GfxBuffer.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>

#include <cassert>
#include <queue>
#include <unordered_map>

namespace Horizon
{
	static ImageLayout toLayout(FrameGraphTextureUsage usage)
	{
		switch (usage)
		{
		case FrameGraphTextureUsage::ColorAttachment:
			return ImageLayout::ColorAttachmentOptimal;
		case FrameGraphTextureUsage::DepthAttachment:
			return ImageLayout::DepthStencilAttachmentOptimal;
		case FrameGraphTextureUsage::ShaderRead:
			return ImageLayout::ShaderReadOnlyOptimal;
		case FrameGraphTextureUsage::StorageRead:
		case FrameGraphTextureUsage::StorageWrite:
			return ImageLayout::General;
		case FrameGraphTextureUsage::TransferSrc:
			return ImageLayout::TransferSrcOptimal;
		case FrameGraphTextureUsage::TransferDst:
			return ImageLayout::TransferDstOptimal;
		}
		return ImageLayout::General;
	}

	static ImageUsage toImageUsage(FrameGraphTextureUsage usage)
	{
		switch (usage)
		{
		case FrameGraphTextureUsage::ColorAttachment:
			return ImageUsage::ColorAttachment;
		case FrameGraphTextureUsage::DepthAttachment:
			return ImageUsage::DepthStencil;
		case FrameGraphTextureUsage::ShaderRead:
			return ImageUsage::Sampled;
		case FrameGraphTextureUsage::StorageRead:
		case FrameGraphTextureUsage::StorageWrite:
			return ImageUsage::Storage;
		case FrameGraphTextureUsage::TransferSrc:
			return ImageUsage::TransferSrc;
		case FrameGraphTextureUsage::TransferDst:
			return ImageUsage::TransferDst;
		}
		return ImageUsage::Sampled;
	}

	static b8 isDepthFormat(ImageFormat fmt)
	{
		return fmt == ImageFormat::D16_UNorm || fmt == ImageFormat::D32_SFloat 
			|| fmt == ImageFormat::D24_UNorm_S8_UInt || fmt == ImageFormat::D32_SFloat_S8_UInt;
	}

	TextureHandle FrameGraph::createTexture(const FrameGraphTextureDesc& desc)
	{
		u32 id = static_cast<u32>(m_textures.size());
		m_textures.push_back({ desc });
		return { id, 0 };
	}

	BufferHandle FrameGraph::createBuffer(const FrameGraphBufferDesc& desc)
	{
		u32 id = static_cast<u32>(m_buffers.size());
		m_buffers.push_back({ desc });
		return { id, 0 };
	}

	TextureHandle FrameGraph::importTexture(std::string_view name, GfxImageView* view)
	{
		u32 id = static_cast<u32>(m_textures.size());
		TextureEntry entry;
		entry.desc.name = std::string(name);
		entry.view = view;
		entry.imported = true;
		m_textures.push_back(std::move(entry));
		return { id, 0 };
	}

	BufferHandle FrameGraph::importBuffer(std::string_view name, GfxBuffer* buffer)
	{
		u32 id = static_cast<u32>(m_buffers.size());
		BufferEntry entry;
		entry.desc.name = std::string(name);
		entry.raw = buffer;
		entry.imported = true;
		m_buffers.push_back(std::move(entry));
		return { id, 0 };
	}


	TextureHandle FrameGraph::writeTexture(TextureHandle handle)
	{
		m_textures[handle.id].version++;
		return { handle.id, m_textures[handle.id].version };
	}

	BufferHandle FrameGraph::writeBuffer(BufferHandle handle)
	{
		m_buffers[handle.id].version++;
		return { handle.id, m_buffers[handle.id].version };
	}

	void FrameGraph::addPass(RenderPass pass)
	{
		m_passes.push_back(std::move(pass));
	}

	void FrameGraph::compile()
	{
		usize n = m_passes.size();
		m_adj.assign(n, {});
		m_inDeg.assign(n, 0);

		auto texKey = [](TextureHandle h) -> u64 { return (u64(h.id) << 32) | h.version; };
		auto bufKey = [](BufferHandle  h) -> u64 { return (u64(h.id) << 32) | h.version; };

		std::unordered_map<u64, usize> texWriter;
		std::unordered_map<u64, usize> bufWriter;

		for (usize i = 0; i < n; i++)
		{
			for (auto& w : m_passes[i].texWrites) texWriter[texKey(w.handle)] = i;
			for (auto& w : m_passes[i].bufWrites) bufWriter[bufKey(w.handle)] = i;
		}

		for (usize j = 0; j < n; j++)
		{
			for (auto& r : m_passes[j].texReads)
				if (auto it = texWriter.find(texKey(r.handle)); it != texWriter.end())
				{
					m_adj[it->second].push_back(j); m_inDeg[j]++;
				}

			for (auto& r : m_passes[j].bufReads)
				if (auto it = bufWriter.find(bufKey(r.handle)); it != bufWriter.end())
				{
					m_adj[it->second].push_back(j); m_inDeg[j]++;
				}
		}

		m_textureUsages.assign(m_textures.size(), static_cast<ImageUsage>(0));
		for (auto& pass : m_passes)
		{
			for (auto& a : pass.texReads)
				m_textureUsages[a.handle.id] = static_cast<ImageUsage>(
					static_cast<u32>(m_textureUsages[a.handle.id]) | static_cast<u32>(toImageUsage(a.usage)));
			for (auto& a : pass.texWrites)
				m_textureUsages[a.handle.id] = static_cast<ImageUsage>(
					static_cast<u32>(m_textureUsages[a.handle.id]) | static_cast<u32>(toImageUsage(a.usage)));
		}

		m_order.clear();
		std::queue<usize> q;
		for (usize i = 0; i < n; i++)
			if (m_inDeg[i] == 0) q.push(i);

		while (!q.empty())
		{
			usize cur = q.front(); q.pop();
			m_order.push_back(cur);
			for (usize nx : m_adj[cur])
				if (--m_inDeg[nx] == 0) q.push(nx);
		}

		assert(m_order.size() == n && "FrameGraph: cycle detected in render pass graph!");
		m_compiled = true;
	}

	void FrameGraph::execute(GfxCommandBuffer& cmd)
	{
		assert(m_compiled && "FrameGraph: compile() must be called before execute()!");

		std::vector<ImageLayout> layouts(m_textures.size(), ImageLayout::Undefined);

		for (usize passIdx : m_order)
		{
			auto& pass = m_passes[passIdx];

			for (auto& tw : pass.texWrites)
			{
				auto& entry = m_textures[tw.handle.id];
				if (entry.imported || entry.owned) 
					continue;

				if (entry.desc.lifetime == ResourceLifetime::Persistent)
				{
					auto it = m_persistentTextures.find(entry.desc.name);
					if (it != m_persistentTextures.end()) 
					{ 
						entry.owned = it->second; 
						continue; 
					}
				}

				GfxImageDesc imgDesc;
				imgDesc
					.setSize({ entry.desc.size.x, entry.desc.size.y, 1 })
					.setMipLevels(entry.desc.mipLevels)
					.setFormat(entry.desc.format)
					.setUsage(m_textureUsages[tw.handle.id])
					.setMemoryUsage(entry.desc.lifetime == ResourceLifetime::Transient
						? MemoryUsage::GpuLazy
						: MemoryUsage::AutoPreferDevice)
					.setMemoryFlags(MemoryAllocation::DedicatedMemory);

				entry.owned = GfxDevice::Get()->CreateImage(imgDesc);

				if (entry.desc.lifetime == ResourceLifetime::Persistent)
					m_persistentTextures[entry.desc.name] = entry.owned;
			}

			for (auto& bw : pass.bufWrites)
			{
				auto& entry = m_buffers[bw.handle.id];
				if (entry.imported || entry.owned) 
					continue;

				if (entry.desc.lifetime == ResourceLifetime::Persistent)
				{
					auto it = m_persistentBuffers.find(entry.desc.name);
					if (it != m_persistentBuffers.end()) 
					{ 
						entry.owned = it->second; 
						continue; 
					}
				}

				GfxBufferDesc bufDesc;
				bufDesc
					.setSize(entry.desc.size)
					.setUsage(entry.desc.usage)
					.setMemoryUsage(MemoryUsage::AutoPreferDevice)
					.setAllocationFlags(MemoryAllocation::DedicatedMemory);

				entry.owned = GfxDevice::Get()->CreateBuffer(bufDesc);

				if (entry.desc.lifetime == ResourceLifetime::Persistent)
					m_persistentBuffers[entry.desc.name] = entry.owned;
			}

			for (auto& tr : pass.texReads)
			{
				GfxImage* img = m_textures[tr.handle.id].getImage();
				if (!img) 
					continue;

				ImageLayout newLayout = toLayout(tr.usage);
				if (layouts[tr.handle.id] == newLayout) 
					continue;

				cmd.ImageBarrier(ImageBarrierDesc{}
					.setImage(img)
					.setOldLayout(layouts[tr.handle.id])
					.setNewLayout(newLayout)
					.setAspect(isDepthFormat(m_textures[tr.handle.id].desc.format)
						? ImageAspect::Depth : ImageAspect::Color));
				layouts[tr.handle.id] = newLayout;
			}

			// Image barriers — write
			for (auto& tw : pass.texWrites)
			{
				GfxImage* img = m_textures[tw.handle.id].getImage();
				if (!img) 
					continue;

				ImageLayout newLayout = toLayout(tw.usage);
				if (layouts[tw.handle.id] == newLayout) 
					continue;

				cmd.ImageBarrier(ImageBarrierDesc{}
					.setImage(img)
					.setOldLayout(layouts[tw.handle.id])
					.setNewLayout(newLayout)
					.setAspect(isDepthFormat(m_textures[tw.handle.id].desc.format)
						? ImageAspect::Depth : ImageAspect::Color));
				layouts[tw.handle.id] = newLayout;
			}

			pass.execute(cmd);
		}
	}

	void FrameGraph::reset()
	{
		for (auto& entry : m_textures)
			if (!entry.imported && entry.desc.lifetime == ResourceLifetime::Transient)
				entry.owned.reset();

		for (auto& entry : m_buffers)
			if (!entry.imported && entry.desc.lifetime == ResourceLifetime::Transient)
				entry.owned.reset();

		m_textures.clear();
		m_buffers.clear();
		m_passes.clear();
		m_adj.clear();
		m_inDeg.clear();
		m_order.clear();
		m_textureUsages.clear();
		m_compiled = false;
	}

	GfxImage* FrameGraph::getImage(TextureHandle handle) const
	{
		return m_textures[handle.id].getImage();
	}

	GfxBuffer* FrameGraph::getBuffer(BufferHandle handle) const
	{
		return m_buffers[handle.id].getBuffer();
	}

	GfxImage* FrameGraph::TextureEntry::getImage() const
	{
		if (owned)
			return owned.get();
		if (view)
			return view->ImageOwner();

		return nullptr;
	}

	GfxBuffer* FrameGraph::BufferEntry::getBuffer() const
	{
		return owned ? owned.get() : raw;
	}

}
