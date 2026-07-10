#pragma once

#include <Runtime/Containers/ScopedLock.h>

#include <Runtime/RHI/GfxTypes.h>
#include <Runtime/RHI/Object/GfxResource.h>

#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/PAL/Sync/CriticalSection.h>

#include <queue>

namespace Horizon
{
	struct GfxTextureDesc;
	class GfxTexture;

	struct GfxBufferDesc;
	class GfxBuffer;

	struct GfxGraphicsPipelineDesc;
	struct GfxComputePipelineDesc;
	class GfxPipeline;

	struct GfxSwapchainDesc;
	class GfxSwapchain;

	class GfxQueue;
	class GfxFence;
	class GfxCommandList;

	class GfxDevice
	{
	public:
		virtual ~GfxDevice() = default;

		virtual void InitializeImGui(GfxQueue* pQueue, GfxTextureFormat fmt = GfxTextureFormat::RGBA8, u32 framesInFlight = MaxFramesInFlight) = 0;
		virtual void ShutdownImGui() = 0;

		virtual GfxTexture* CreateTexture(const GfxTextureDesc& desc) = 0;
		virtual GfxBuffer* CreateBuffer(const GfxBufferDesc& desc) = 0;
		virtual GfxPipeline* CreatePipeline(const GfxGraphicsPipelineDesc& desc) = 0;
		virtual GfxPipeline* CreatePipeline(const GfxComputePipelineDesc& desc) = 0;

		virtual GfxQueue* CreateQueue(GfxQueueType type) = 0;
		virtual GfxFence* CreateFence() = 0;
		virtual GfxCommandList* CreateCommandList(GfxQueueType type) = 0;
		virtual GfxSwapchain* CreateSwapchain(const GfxSwapchainDesc& desc, GfxQueue* presentQueue) = 0;

		virtual void FreeDescriptor(GfxDescriptorHeapType type, u32 index) = 0;

		void EnqueueDelete(GfxResource* pResource);
		void FlushPendingDeletes(u64 currentFrame);
		void SetFrameIndex(u64 idx);

	private:
		struct Pending
		{
			GfxResource* pResource;
			u64 retireFrame;
		};

		std::queue<Pending> m_pending;
		PAL::CriticalSection m_mutex;
		PAL::Atomic<u64> m_frameIndex = 0;
	};

	GfxDevice* CreateGfxDevice();
}