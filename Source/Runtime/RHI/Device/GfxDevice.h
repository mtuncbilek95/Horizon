#pragma once

#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Definitions/Allocator.h>

#include <Runtime/RHI/GfxTypes.h>
#include <Runtime/RHI/Object/GfxObject.h>

#include <Runtime/PAL/Sync/Atomic.h>
#include <Runtime/PAL/Sync/CriticalSection.h>

#include <queue>

namespace Horizon
{
	struct GfxTextureDesc;
	class GfxTexture;

	struct GfxBufferDesc;
	class GfxBuffer;

	class GfxSampler;

	struct GfxGraphicsPipelineDesc;
	struct GfxComputePipelineDesc;
	class GfxPipeline;

	struct GfxSwapchainDesc;
	class GfxSwapchain;

	class GfxDescriptorHeap;
	class GfxQueue;
	class GfxFence;
	class GfxCommandList;

	struct GfxDescriptorBudget
	{
		u32 resourceCount = 1 << 16;
		u32 samplerCount = 1 << 8;
		u32 colorCount = 1 << 10;
		u32 depthCount = 1 << 8;
	};

	struct GfxDeviceDesc
	{
		GfxDescriptorBudget descriptorBudget{};
		u32 framesInFlight = 2;
		u64 vramSoftBudget = 0;
	};

	struct GfxAdapterInfo
	{
		char name[128] = {};
		u64 dedicatedVideoMemory = 0;
		u64 sharedSystemMemory = 0;
		u32 vendorId = 0;
		u32 deviceId = 0;
	};

	struct GfxMemoryStats
	{
		u64 budget = 0;
		u64 usage = 0;
	};

	class GfxDevice
	{
	public:
		virtual ~GfxDevice() = default;

		virtual void InitializeImGui(GfxQueue* pQueue, GfxTextureFormat fmt = GfxTextureFormat::RGBA8) = 0;
		virtual void ShutdownImGui() = 0;

		virtual GfxTexture* CreateTexture(const GfxTextureDesc& desc) = 0;
		virtual GfxBuffer* CreateBuffer(const GfxBufferDesc& desc) = 0;
		virtual GfxSampler* CreateSampler(const GfxSamplerDesc& desc) = 0;
		virtual GfxPipeline* CreatePipeline(const GfxGraphicsPipelineDesc& desc) = 0;
		virtual GfxPipeline* CreatePipeline(const GfxComputePipelineDesc& desc) = 0;

		virtual GfxQueue* CreateQueue(GfxQueueType type) = 0;
		virtual GfxFence* CreateFence() = 0;
		virtual GfxCommandList* CreateCommandList(GfxQueueType type) = 0;
		virtual GfxSwapchain* CreateSwapchain(const GfxSwapchainDesc& desc, GfxQueue* pPresentQueue) = 0;

		virtual GfxDescriptorHeap* GetDescriptorHeap(GfxDescriptorHeapType type) = 0;
		virtual GfxAdapterInfo GetAdapterInfo() const = 0;
		virtual GfxMemoryStats QueryMemoryStats() = 0;

		const GfxDeviceDesc& GetDesc() const { return m_desc; }

		void EnqueueDelete(GfxObject* pObject)
		{
			ScopedLock lock(m_mutex);

			m_pending.push({ pObject, m_frameIndex.Load() });
		}

		void FlushPendingDeletes(u64 currentFrame)
		{
			ScopedLock lock(m_mutex);

			while (!m_pending.empty() && m_pending.front().retireFrame + m_desc.framesInFlight <= currentFrame)
			{
				Memory::Allocator::Delete(m_pending.front().pObject);
				m_pending.pop();
			}
		}

		void SetFrameIndex(u64 idx) { m_frameIndex.Store(idx); }

	protected:
		GfxDeviceDesc m_desc{};

	private:
		struct Pending
		{
			GfxObject* pObject;
			u64 retireFrame;
		};

		std::queue<Pending> m_pending;
		PAL::CriticalSection m_mutex;
		PAL::Atomic<u64> m_frameIndex = 0;
	};

	GfxDevice* CreateGfxDevice(const GfxDeviceDesc& desc);
}