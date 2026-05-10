#include "PassBuilder.h"

#include <Engine/World/FrameGraph/FrameGraph.h>

namespace Horizon
{
	PassBuilder::PassBuilder(FrameGraph& graph, u32 passIndex)
		: m_graph(graph), m_passIndex(passIndex)
	{}

	RenderTargetHandle PassBuilder::Create(const RenderTargetDesc& desc)
	{
		RenderTargetHandle handle = m_graph.CreateResource(desc);
		m_graph.m_passes[m_passIndex].creates.push_back(handle);
		return handle;
	}

	RenderTargetHandle PassBuilder::Read(RenderTargetHandle handle)
	{
		m_graph.RegisterRead(m_passIndex, handle);
		return handle;
	}

	RenderTargetHandle PassBuilder::Write(RenderTargetHandle handle)
	{
		m_graph.RegisterWrite(m_passIndex, handle);
		handle.version++;
		return handle;
	}
}