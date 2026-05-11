#include "PassResources.h"

#include <Engine/World/FrameGraph/FrameGraph.h>

namespace Horizon
{
	PassResources::PassResources(const std::vector<ResourceEntry>& resources)
		: m_resources(resources)
	{}

	GfxImageView* PassResources::Get(RenderTargetHandle handle) const
	{
		auto& entry = m_resources[handle.idx];
		if (entry.imported)
			return entry.importedView;
		return entry.resource->view.get();
	}
}