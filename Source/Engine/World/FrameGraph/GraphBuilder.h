#pragma once

#include <Engine/World/FrameGraph/ResourceHandle.h>
#include <Engine/World/FrameGraph/RenderTargetDesc.h>

namespace Horizon
{
	class FrameGraph;

	class GraphBuilder
	{
	public:
		GraphBuilder(const std::string& passName, FrameGraph& graph, u32 passIndex);

		RenderTargetHandle Create(const std::string& name, const RenderTargetDesc& desc);
		RenderTargetHandle Read(RenderTargetHandle handle);
		RenderTargetHandle Write(RenderTargetHandle handle);
		RenderTargetHandle ReadFrom(const std::string& passName, const std::string& resourceName);
		RenderTargetHandle WriteToComposite();

	private:
		FrameGraph& m_graph;
		std::string m_passName;
		u32 m_passIndex;
	};
}