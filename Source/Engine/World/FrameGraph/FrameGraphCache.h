#pragma once

#include <Engine/World/FrameGraph/RenderTargetDesc.h>
#include <unordered_map>

namespace Horizon
{
	class GfxDevice;

	class FrameGraphCache
	{
	public:
		FrameGraphCache(GfxDevice* pDevice);
		~FrameGraphCache();

		RenderTargetEntry& GetOrCreate(const RenderTargetDesc& desc);

	private:
		RenderTargetEntry CreateRenderTarget(const RenderTargetDesc& desc);

	private:
		GfxDevice* m_device;
		std::unordered_multimap<RenderTargetDesc, RenderTargetEntry> m_caches;
	};
}