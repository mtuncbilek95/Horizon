#include "PresentationSystem.h"

#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/Graphics/RHI/Sync/GfxSemaphore.h>
#include <Runtime/Graphics/RHI/Sync/GfxFence.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>

#include <Engine/Graphics/GraphicsSystem.h>

namespace Horizon
{
	SystemReport PresentationSystem::OnInitialize()
	{
		auto& graphicsSystem = RequestSystem<GraphicsSystem>();
		m_swapchain = graphicsSystem.Swapchain();
		m_presentationQueue = graphicsSystem.GraphicsQueue();

		return SystemReport();
	}

	void PresentationSystem::OnSync()
	{
		auto& graphicsSystem = RequestSystem<GraphicsSystem>();

		std::vector<GfxCommandBuffer*> cmdBuffers;
		for (const auto& request : m_pendingPresentRequests)
			cmdBuffers.push_back(request.cmdBuffer);

		GfxSemaphore* currRenderSemaphore = graphicsSystem.CurrentRenderFinishedSem();

		m_presentationQueue->Submit(cmdBuffers, {}, { currRenderSemaphore }, nullptr, PipelineStageFlags::ColorAttachment);

		m_swapchain->Present({ currRenderSemaphore });
		graphicsSystem.AdvanceFrame();

		m_pendingPresentRequests.clear();
	}

	void PresentationSystem::OnFinalize()
	{
	}

	void PresentationSystem::RequestPresent(const PresentRequest& request)
	{
		m_pendingPresentRequests.push_back(request);
	}
}
