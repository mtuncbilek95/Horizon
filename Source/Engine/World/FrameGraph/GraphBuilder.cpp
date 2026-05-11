#include "GraphBuilder.h"

#include <Engine/World/FrameGraph/FrameGraph.h>

namespace Horizon
{
	GraphBuilder::GraphBuilder(const std::string& passName, FrameGraph& graph, u32 passIndex) : m_passName(passName), 
		m_graph(graph), m_passIndex(passIndex)
	{}

	RenderTargetHandle GraphBuilder::Create(const std::string& name, const RenderTargetDesc& desc)
	{
		RenderTargetHandle handle = m_graph.CreateResource(name, desc);
		std::string key = m_passName + "::" + name;
		m_graph.m_handleRegistry[key] = handle;
		return handle;
	}

	RenderTargetHandle GraphBuilder::Read(RenderTargetHandle handle)
	{
		m_graph.RegisterRead(m_passIndex, handle);
		return handle;
	}

	RenderTargetHandle GraphBuilder::Write(RenderTargetHandle handle)
	{
		m_graph.RegisterWrite(m_passIndex, handle);
		handle.version++;
		return handle;
	}

	RenderTargetHandle GraphBuilder::ReadFrom(const std::string& passName, const std::string& resourceName)
	{
		std::string key = passName + "::" + resourceName;
		auto it = m_graph.m_handleRegistry.find(key);
		if (it == m_graph.m_handleRegistry.end())
			return RenderTargetHandle();

		RenderTargetHandle handle = it->second;
		m_graph.RegisterRead(m_passIndex, handle);
		return handle;
	}

	RenderTargetHandle GraphBuilder::WriteToComposite()
	{
		RenderTargetHandle handle = m_graph.m_backbufferHandle;
		m_graph.RegisterWrite(m_passIndex, handle);
		handle.version++;
		return handle;
	}
}