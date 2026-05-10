#include "PassResources.h"

#include <Engine/World/FrameGraph/FrameGraph.h>

namespace Horizon
{
	PassResources::PassResources(const std::vector<ResourceEntry>& resources)
		: m_resources(resources)
	{}

	GfxImageView* PassResources::Get(RenderTargetHandle handle) const
	{
		return m_resources[handle.idx].resource->view.get();
	}
}