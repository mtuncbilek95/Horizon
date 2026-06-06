#include "FrameGraphBuilder.h"

#include <Engine/FrameGraph/FrameGraph.h>

namespace Horizon
{
	FrameGraphBuilder::FrameGraphBuilder(FrameGraph& graph, FrameGraphPass& pass) : m_graph(graph), m_pass(pass)
	{}

	FGTextureHandle FrameGraphBuilder::CreateTexture(const GfxTextureDesc& desc, std::string_view name)
	{
		u32 resourceIndex = m_graph.RegisterTransientTexture(desc, name);
		return FGTextureHandle::make(resourceIndex, 0);
	}

	FGBufferHandle FrameGraphBuilder::CreateBuffer(const GfxBufferDesc& desc, std::string_view name)
	{
		u32 resourceIndex = m_graph.RegisterTransientBuffer(desc, name);
		return FGBufferHandle::make(resourceIndex, 0);
	}

	FGTextureHandle FrameGraphBuilder::Read(FGTextureHandle texture, GfxResourceState state)
	{
		FrameGraphAccess access = {};
		access.type = FrameGraphResourceType::Texture;
		access.resourceIndex = texture.index();
		access.version = texture.generation();
		access.state = state;

		m_pass.m_reads.push_back(access);
		return texture;
	}

	FGBufferHandle FrameGraphBuilder::Read(FGBufferHandle buffer, GfxResourceState state)
	{
		FrameGraphAccess access = {};
		access.type = FrameGraphResourceType::Buffer;
		access.resourceIndex = buffer.index();
		access.version = buffer.generation();
		access.state = state;

		m_pass.m_reads.push_back(access);
		return buffer;
	}

	FGTextureHandle FrameGraphBuilder::Write(FGTextureHandle texture, GfxResourceState state)
	{
		u32 resourceIndex = texture.index();
		u32 newVersion = m_graph.NextTextureVersion(resourceIndex);

		FrameGraphAccess access = {};
		access.type = FrameGraphResourceType::Texture;
		access.resourceIndex = resourceIndex;
		access.version = newVersion;
		access.state = state;

		m_pass.m_writes.push_back(access);
		return FGTextureHandle::make(resourceIndex, newVersion);
	}

	FGBufferHandle FrameGraphBuilder::Write(FGBufferHandle buffer, GfxResourceState state)
	{
		u32 resourceIndex = buffer.index();
		u32 newVersion = m_graph.NextBufferVersion(resourceIndex);

		FrameGraphAccess access = {};
		access.type = FrameGraphResourceType::Buffer;
		access.resourceIndex = resourceIndex;
		access.version = newVersion;
		access.state = state;

		m_pass.m_writes.push_back(access);
		return FGBufferHandle::make(resourceIndex, newVersion);
	}

	FGTextureHandle FrameGraphBuilder::SetColorAttachment(u32 slot, FGTextureHandle texture,
		GfxLoadOp load, GfxStoreOp store)
	{
		if (load == GfxLoadOp::Load)
		{
			FrameGraphAccess readAccess = {};
			readAccess.type = FrameGraphResourceType::Texture;
			readAccess.resourceIndex = texture.index();
			readAccess.version = texture.generation();
			readAccess.state = GfxResourceState::RenderTarget;
			m_pass.m_reads.push_back(readAccess);
		}

		FGTextureHandle written = Write(texture, GfxResourceState::RenderTarget);

		FrameGraphColorAttachment& attachment = m_pass.m_colorAttachments[slot];
		attachment.resourceIndex = texture.index();
		attachment.load = load;
		attachment.store = store;

		if (slot + 1 > m_pass.m_colorCount)
			m_pass.m_colorCount = slot + 1;

		return written;
	}

	FGTextureHandle FrameGraphBuilder::SetDepthAttachment(FGTextureHandle texture,
		GfxLoadOp load, GfxStoreOp store)
	{
		if (load == GfxLoadOp::Load)
		{
			FrameGraphAccess readAccess = {};
			readAccess.type = FrameGraphResourceType::Texture;
			readAccess.resourceIndex = texture.index();
			readAccess.version = texture.generation();
			readAccess.state = GfxResourceState::DepthRead;
			m_pass.m_reads.push_back(readAccess);
		}

		FGTextureHandle written = Write(texture, GfxResourceState::DepthWrite);

		FrameGraphDepthAttachment& attachment = m_pass.m_depthAttachment;
		attachment.resourceIndex = texture.index();
		attachment.load = load;
		attachment.store = store;
		attachment.enabled = true;

		return written;
	}

	void FrameGraphBuilder::SetSideEffect()
	{
		m_pass.m_hasSideEffect = true;
	}
}