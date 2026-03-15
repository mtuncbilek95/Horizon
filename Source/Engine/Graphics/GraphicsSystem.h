#pragma once

#include <Engine/System/System.h>
#include <Engine/Graphics/RenderRequest.h>

namespace Horizon
{
	class GfxInstance;
	class GfxDevice;
	class GfxQueue;
	class GfxSwapchain;
	class GfxCommandPool;
	class GfxCommandBuffer;
	class GfxFence;
	class GfxSemaphore;
	class GfxRenderPass;
	class GfxFramebuffer;

	class EditorRenderer;

	class GraphicsSystem : public System<GraphicsSystem>
	{
	public:
		GraphicsSystem();
		virtual ~GraphicsSystem() = default;

		GfxInstance* Instance() const { return m_instance.get(); }
		GfxDevice* Device() const { return m_device.get(); }

		GfxQueue* GraphicsQueue() const { return m_graphicsQueue.get(); }
		GfxQueue* ComputeQueue() const { return m_computeQueue.get(); }
		GfxQueue* TransferQueue() const { return m_transferQueue.get(); }

		GfxSwapchain* Swapchain() const { return m_swapchain.get(); }
		GfxRenderPass* PresentationPass() const { return m_presentationPass.get(); }
		GfxFramebuffer* PresentationFramebuffer() const { return m_presentationFramebuffer.get(); }

		GfxCommandBuffer* CurrentGraphicsCommand() const { return m_graphicsCmds[m_currentFrame].get(); }
		GfxFence* CurrentFrameFence() const { return m_frameFences[m_currentFrame].get(); }
		GfxSemaphore* CurrentRenderFinishedSem() const { return m_sceneFinishedSems[m_currentImageIndex].get(); }

		u32 CurrentFrameIndex() const { return m_currentFrame; }
		u32 CurrentImageIndex() const { return m_currentImageIndex; }

		SystemReport OnInitialize() override;
		void OnSync() override;
		void OnFinalize() override;

		void AdvanceFrame();
		void Submit(RenderRequest request) { m_renderRequests.push_back(std::move(request)); }

	private:
		std::shared_ptr<GfxInstance> m_instance;
		std::shared_ptr<GfxDevice> m_device;

		std::shared_ptr<GfxQueue> m_graphicsQueue;
		std::shared_ptr<GfxQueue> m_computeQueue;
		std::shared_ptr<GfxQueue> m_transferQueue;

		std::shared_ptr<GfxSwapchain> m_swapchain;
		std::shared_ptr<GfxCommandPool> m_graphicsCmdPool;
		std::shared_ptr<GfxRenderPass> m_presentationPass;
		std::shared_ptr<GfxFramebuffer> m_presentationFramebuffer;

		std::vector<std::shared_ptr<GfxCommandBuffer>> m_graphicsCmds;
		std::vector<std::shared_ptr<GfxFence>> m_frameFences;
		std::vector<std::shared_ptr<GfxSemaphore>> m_sceneFinishedSems;

		std::shared_ptr<EditorRenderer> m_editor;

		std::vector<RenderRequest> m_renderRequests;

		u32 m_currentFrame;
		u32 m_currentImageIndex;
	};
}
