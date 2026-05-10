#pragma once

#include <Engine/World/FrameGraph/ResourceHandle.h>
#include <Engine/World/FrameGraph/RenderTargetDesc.h>
#include <Engine/World/FrameGraph/RenderTargetEntry.h>
#include <Engine/World/FrameGraph/PassBuilder.h>
#include <Engine/World/FrameGraph/PassResources.h>
#include <Engine/World/FrameGraph/FrameGraphCache.h>
#include <Engine/Presentation/PresentationObjects.h>

#include <functional>
#include <string>

namespace Horizon
{
	class GfxCommandBuffer;

	struct ResourceEntry
	{
		RenderTargetDesc desc;
		u32 writer = u32_max;
		std::vector<u32> readers;
		RenderTargetEntry* resource = nullptr;
		b8 imported = false;
	};

	class FrameGraph
	{
	public:
		void SetGraphCache(std::unique_ptr<FrameGraphCache> cache);

		RenderTargetHandle Import(const CompositePresentObject& presentObj);

		void AddPass(const std::string& name,
			std::function<void(PassBuilder&)> setup,
			std::function<void(GfxCommandBuffer*, const PassResources&)> execute);

		void Compile();
		void Execute(GfxCommandBuffer* cmd);
		void Reset();

	private:
		friend class PassBuilder;

		RenderTargetHandle CreateResource(const RenderTargetDesc& desc);
		void RegisterRead(u32 passIndex, RenderTargetHandle handle);
		void RegisterWrite(u32 passIndex, RenderTargetHandle handle);

		struct PassEntry
		{
			std::string name;
			std::function<void(GfxCommandBuffer*, const PassResources&)> execute;
			std::vector<RenderTargetHandle> reads;
			std::vector<RenderTargetHandle> writes;
			std::vector<RenderTargetHandle> creates;
		};

		std::vector<ResourceEntry> m_resources;
		std::vector<PassEntry> m_passes;
		std::vector<u32> m_executionOrder;
		std::unique_ptr<FrameGraphCache> m_cache;
		std::vector<RenderTargetEntry> m_importedResources;
	};
}