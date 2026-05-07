#pragma once

#include <Application/Engine/System.h>
#include <Application/Systems/Renderer/RenderBatch.h>

#include <memory>

namespace Horizon
{
	class GfxInstance;
	class GfxDevice;
	class GfxQueue;
	class GfxSwapchain;
	class GfxFence;
	class GfxSemaphore;
	class GfxCommandBuffer;
	class GfxCommandPool;

	class RenderSystem final : public System<RenderSystem>
	{
	public:
		RenderSystem() = default;
		~RenderSystem() = default;

		GfxInstance& Instance() const { return *m_instance.get(); }
		GfxDevice& Device() const { return *m_device.get(); }
		GfxQueue& GraphicsQueue() const { return *m_graphicsQueue.get(); }
		GfxQueue& ComputeQueue() const { return *m_computeQueue.get(); }
		GfxQueue& TransferQueue() const { return *m_transferQueue.get(); }

		RenderBatch& GetBatch() { return m_renderBatch; }

	private:
		EngineReport OnInitialize() final;
		void OnSync() final;
		void OnFinalize() final;

	private:
		std::shared_ptr<GfxInstance> m_instance;
		std::shared_ptr<GfxDevice> m_device;

		std::shared_ptr<GfxQueue> m_graphicsQueue;
		std::shared_ptr<GfxQueue> m_computeQueue;
		std::shared_ptr<GfxQueue> m_transferQueue;

		std::shared_ptr<GfxSwapchain> m_swapchain;

		std::shared_ptr<GfxCommandPool> m_cmdPool;
		std::vector<std::shared_ptr<GfxCommandBuffer>> m_cmdBuffers;
		std::vector<std::shared_ptr<GfxSemaphore>> m_imageAvailableSemaphores;
		std::vector<std::shared_ptr<GfxSemaphore>> m_renderFinishedSemaphores;
		std::vector<std::shared_ptr<GfxFence>> m_fences;

		u32 m_frameIndex = 0;

		RenderBatch m_renderBatch;
	};
}