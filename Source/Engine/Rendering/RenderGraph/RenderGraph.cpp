#include "RenderGraph.h"

namespace Horizon::Engine
{
	RenderGraph::RenderGraph(Engine* pEngine)
	{
	}

	RenderGraph::~RenderGraph()
	{
	}

	b8 RenderGraph::OnInitialize()
	{
		return true;
	}

	void RenderGraph::OnExecute()
	{
	}

	void RenderGraph::OnFinalize()
	{
	}

	void RenderGraph::OnLibraryRegistered(const ReflectionLibrary& library)
	{
	}

	void RenderGraph::OnLibraryUnregistered(const ReflectionLibrary& library)
	{
	}
}