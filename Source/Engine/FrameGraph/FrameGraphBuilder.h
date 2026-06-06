#pragma once

#include <Engine/FrameGraph/FrameGraphHandle.h>
#include <Engine/Graphics/RHI/GfxCommandList.h>

#include <string_view>

namespace Horizon
{
	struct GfxTextureDesc;
	struct GfxBufferDesc;

	class FrameGraph;
	class FrameGraphPass;

	/*
	 * A builder system for FrameGraph
	 */
	class FrameGraphBuilder
	{
	public:
		FrameGraphBuilder(FrameGraph& graph, FrameGraphPass& pass);

		// Transient resource creation
		// Those below is created/destroyed inside of the graph
		FGTextureHandle CreateTexture(const GfxTextureDesc& desc, std::string_view name);
		FGBufferHandle CreateBuffer(const GfxBufferDesc& desc, std::string_view name);

		// General dependency
		// For either compute or copy
		FGTextureHandle Read(FGTextureHandle texture, GfxResourceState state);
		FGBufferHandle Read(FGBufferHandle buffer, GfxResourceState state);

		FGTextureHandle Write(FGTextureHandle texture, GfxResourceState state);
		FGBufferHandle Write(FGBufferHandle buffer, GfxResourceState state);

		// Render pass extensions 
		// Used for color and depth targets pin
		FGTextureHandle SetColorAttachment(u32 slot, FGTextureHandle texture, 
			GfxLoadOp load = GfxLoadOp::Clear, GfxStoreOp store = GfxStoreOp::Store);
		FGTextureHandle SetDepthAttachment(FGTextureHandle texture, 
			GfxLoadOp load = GfxLoadOp::Clear, GfxStoreOp store = GfxStoreOp::Store);

		void SetSideEffect();

	private:
		FrameGraph& m_graph;
		FrameGraphPass& m_pass;
	};
}