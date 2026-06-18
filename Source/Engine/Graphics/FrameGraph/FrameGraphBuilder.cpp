#include "FrameGraphBuilder.h"

#include <Engine/Graphics/FrameGraph/FrameGraph.h>

namespace Horizon
{
	FGTextureHandle FrameGraphBuilder::CreateTexture(const char* name, const GfxTextureDesc& desc)
	{
		return m_graph.CreateTexture(name, desc);
	}

	FGTextureHandle FrameGraphBuilder::Read(FGTextureHandle handle, GfxResourceState state)
	{
		m_graph.RegisterRead(m_passIndex, handle, state);
		return handle;
	}

	FGTextureHandle FrameGraphBuilder::Write(FGTextureHandle handle, GfxResourceState state)
	{
		m_graph.RegisterWrite(m_passIndex, handle, state);
		return handle;
	}
}