#pragma once

#include <Engine/Graphics/FrameGraph/FrameGraphHandle.h>
#include <Runtime/Graphics/RHI/Texture/GfxTexture.h> 

namespace Horizon
{
	class FrameGraph;
	class FrameGraphBuilder
	{
	public:
		FrameGraphBuilder(FrameGraph& graph, u32 passIndex) : m_graph(graph), 
			m_passIndex(passIndex) 
		{}

		FGTextureHandle CreateTexture(const char* name, const GfxTextureDesc& desc);
		FGTextureHandle Read(FGTextureHandle handle, GfxResourceState state);
		FGTextureHandle Write(FGTextureHandle handle, GfxResourceState state);
	private:
		FrameGraph& m_graph;
		u32 m_passIndex;
	};
}