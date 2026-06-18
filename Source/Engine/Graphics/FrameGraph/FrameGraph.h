#pragma once

#include <Runtime/Graphics/RHI/Texture/GfxTexture.h>
#include <Runtime/Graphics/RHI/GfxTypes.h>

#include <Engine/Graphics/FrameGraph/FrameGraphHandle.h>
#include <Engine/Graphics/FrameGraph/FrameGraphResources.h>

#include <functional>
#include <vector>
#include <string>

namespace Horizon
{
	class GfxQueue;
	class GfxCommandList;

	class FrameGraphBuilder;
	class TransientResourcePool;

	class JobModule;
	class FrameContext;

	using FrameGraphSetupFn = std::function<void(FrameGraphBuilder&)>;
	using FrameGraphExecuteFn = std::function<void(GfxCommandList*, const FrameGraphResources&)>;

	class FrameGraph
	{
		friend class FrameGraphBuilder;

		struct ResourceAccess
		{
			u32 resource = kInvalid32;
			GfxResourceState state = GfxResourceState::Common;
		};

		struct FGBarrier
		{
			u32 resource = kInvalid32;
			GfxResourceState before = GfxResourceState::Common;
			GfxResourceState after = GfxResourceState::Common;
		};

		struct ResourceNode
		{
			std::string name;
			GfxTextureDesc desc = {};
			GfxTexture* imported = nullptr;

			GfxResourceState initialState = GfxResourceState::Common;
			GfxResourceState finalState = GfxResourceState::Common;

			i32 producer = -1;
			u32 refCount = 0;
			i32 firstUse = -1, lastUse = -1;

			b8 IsImported() const { return imported != nullptr; }
		};

		struct PassNode
		{
			std::string name;
			FrameGraphExecuteFn execute;
			std::vector<ResourceAccess> reads;
			std::vector<ResourceAccess> writes;
			GfxQueueType queue = GfxQueueType::Graphics;

			u32 refCount = 0;
			b8 culled = false;
			b8 hasSideEffect = false;

			std::vector<FGBarrier> entryBarriers;
		};

	public:
		FGTextureHandle ImportTexture(const char* name, GfxTexture* texture, GfxResourceState initState, GfxResourceState finalState);

		void AddPass(const char* name, FrameGraphSetupFn setup, FrameGraphExecuteFn execute);
		void MarkSideEffect();

		void Compile(TransientResourcePool& pool);
		void Execute(JobModule& jobs, FrameContext& frame, GfxQueue& queue);
		void Clear();

	private:
		FGTextureHandle CreateTexture(const char* name, const GfxTextureDesc& desc);
		void RegisterRead(u32 passIndex, FGTextureHandle handle, GfxResourceState state);
		void RegisterWrite(u32 passIndex, FGTextureHandle handle, GfxResourceState state);
		void ResolveResources(TransientResourcePool& pool);

		void RecordPass(u32 orderPos, FrameContext& frame);
		void IssueBarriers(GfxCommandList* cmd, const std::vector<FGBarrier>& barriers);

		void Cull();
		void BuildOrder();
		void ComputeLifetimes();
		void ComputeBarriers();

	private:
		std::vector<ResourceNode> m_resources;
		std::vector<PassNode> m_passes;

		std::vector<u32> m_order;
		std::vector<FGBarrier> m_exitBarriers;
		std::vector<GfxTexture*> m_physicalTextures;

		std::vector<GfxCommandList*> m_recordedLists;
	};
}