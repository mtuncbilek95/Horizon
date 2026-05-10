#include "FrameGraph.h"

#include <Engine/World/FrameGraph/FrameGraphCache.h>

namespace Horizon
{

	void FrameGraph::ResolveCompositePass(const CompositePresentObject& passObj)
	{
		m_composite = passObj;
	}

	void FrameGraph::SetGraphCache(std::unique_ptr<FrameGraphCache> cache)
	{
		m_cache = std::move(cache);
	}

}