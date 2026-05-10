#pragma once

#include <Engine/World/FrameGraph/ResourceHandle.h>

#include <vector>

namespace Horizon
{
	class GfxImageView;
	struct ResourceEntry;

	class PassResources
	{
	public:
		PassResources(const std::vector<ResourceEntry>& resources);

		GfxImageView* Get(RenderTargetHandle handle) const;

	private:
		const std::vector<ResourceEntry>& m_resources;
	};
}