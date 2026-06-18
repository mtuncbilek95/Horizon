#pragma once

#include <Runtime/Graphics/RHI/GfxTypes.h>
#include <Runtime/Graphics/RHI/Object/GfxResource.h>
#include <Runtime/Graphics/RHI/Object/GfxPointer.h>

#include <queue>
#include <atomic>
#include <mutex>
#include <memory>

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

		virtual GfxPointer<GfxTexture> CreateTexture(const GfxTextureDesc& desc) = 0;
		virtual GfxPointer<GfxBuffer> CreateBuffer(const GfxBufferDesc& desc) = 0;
		virtual GfxPointer<GfxPipeline> CreatePipeline(const GfxGraphicsPipelineDesc& desc) = 0;
		virtual GfxPointer<GfxPipeline> CreatePipeline(const GfxComputePipelineDesc& desc) = 0;

		virtual std::unique_ptr<GfxQueue> CreateQueue(GfxQueueType type) = 0;
		virtual std::unique_ptr<GfxFence> CreateFence() = 0;
		virtual std::unique_ptr<GfxCommandList> CreateCommandList(GfxQueueType type) = 0;
		virtual std::unique_ptr<GfxSwapchain> CreateSwapchain(const GfxSwapchainDesc& desc, GfxQueue* presentQueue) = 0;

		virtual void FreeDescriptor(GfxDescriptorHeapType type, u32 index) = 0;

		virtual void InitializeImGui(void* pAPIHandle, GfxQueue* graphicsQueue) = 0;
		virtual void NewFrameImGui() = 0;
		virtual void ShutdownImGui() = 0;

		// Bindless, shader-visible texture id for displaying a texture via ImGui::Image.
		virtual u64 GetImGuiTextureId(GfxTexture* texture) = 0;

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
		std::mutex m_mutex;
		std::atomic<u64> m_frameIndex = 0;
	};

	std::unique_ptr<GfxDevice> CreateGfxDevice();
}