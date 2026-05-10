#pragma once

#include <Engine/World/FrameGraph/ResourceHandle.h>
#include <Engine/World/FrameGraph/RenderTargetDesc.h>

namespace Horizon
{
	class FrameGraph;

	class PassBuilder
	{
	public:
		PassBuilder(FrameGraph& graph, u32 passIndex);

		RenderTargetHandle Create(const RenderTargetDesc& desc);
		RenderTargetHandle Read(RenderTargetHandle handle);
		RenderTargetHandle Write(RenderTargetHandle handle);

	private:
		FrameGraph& m_graph;
		u32 m_passIndex;
	};
}