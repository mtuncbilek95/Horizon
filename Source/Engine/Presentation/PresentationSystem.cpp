#include "PresentationSystem.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>
#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/Graphics/RHI/Sync/GfxFence.h>
#include <Runtime/Graphics/RHI/Sync/GfxSemaphore.h>

#include <Engine/Cache/CacheContext.h>
#include <Engine/Graphics/GfxContext.h>
#include <Engine/Window/WindowSystem.h>

namespace Horizon
{
	CompositePresentObject PresentationSystem::AcquireNextImage()
	{
		m_fences[m_frameIndex]->WaitIdle();
		m_fences[m_frameIndex]->Reset();

		u32 imageIndex = m_swapchain->AcquireNextImage(m_imageAvailableSemaphores[m_frameIndex].get(), nullptr);
		auto* swapImageView = m_swapchain->ImageView(imageIndex);

		return CompositePresentObject()
			.setImageCount(m_swapchain->ImageCount())
			.setFrameIndex(m_frameIndex)
			.setImageIndex(imageIndex)
			.setImageView(swapImageView);
	}

	void PresentationSystem::SubmitPresent(const CompositeSubmitObject& obj)
	{
		m_graphicsQueue->Submit(
			obj.cmdBuffers,
			{ m_imageAvailableSemaphores[m_frameIndex].get() },
			{ m_renderFinishedSemaphores[obj.imageIndex].get() },
			m_fences[m_frameIndex].get(),
			PipelineStageFlags::ColorAttachment);

		m_swapchain->Present({ m_renderFinishedSemaphores[obj.imageIndex].get() });
		m_frameIndex = (m_frameIndex + 1) % MaxFramesInFlight;
	}

	EngineReport PresentationSystem::OnInitialize()
	{
		auto& gfxCtx = RequestContext<GfxContext>();
		auto& wndSys = RequestSystem<WindowSystem>();

		auto& cacheCtx = RequestContext<CacheContext>();
		auto& gfxCache = cacheCtx.GetGfxCache();

		m_graphicsQueue = &gfxCtx.GraphicsQueue();
		m_swapchain = gfxCtx.Device().CreateSwapchain(GfxSwapchainDesc()
			.setFormat(ImageFormat::R8G8B8A8_UNorm)
			.setGraphicsQueue(m_graphicsQueue)
			.setImageCount(gfxCache.imageCount)
			.setImageSize(wndSys.WindowSize())
			.setPresentMode(gfxCache.presentType)
			.setWindowAPI(wndSys.APIWindow())
			.setWindowHandler(wndSys.Handle())
			.setWindowInstance(wndSys.Instance()));
		if (!m_swapchain)
			return EngineReport("RHI", "failed initializing GfxSwapchain");

		for (size_t i = 0; i < MaxFramesInFlight; i++)
			m_fences.push_back(gfxCtx.Device().CreateSyncFence(GfxFenceDesc().setSignal(true)));

		for (size_t i = 0; i < MaxFramesInFlight; i++)
			m_imageAvailableSemaphores.push_back(gfxCtx.Device().CreateSyncSemaphore(GfxSemaphoreDesc()));

		for (size_t i = 0; i < m_swapchain->ImageCount(); i++)
			m_renderFinishedSemaphores.push_back(gfxCtx.Device().CreateSyncSemaphore(GfxSemaphoreDesc()));

		return EngineReport();
	}

	void PresentationSystem::OnSync()
	{}

	void PresentationSystem::OnFinalize()
	{
		auto& gfxContext = RequestContext<GfxContext>();
		gfxContext.Device().WaitIdle();

		auto& cacheCtx = RequestContext<CacheContext>();
		auto& gfxCache = cacheCtx.GetGfxCache();

		gfxCache.presentType = m_swapchain->PresentationMode();
		gfxCache.imageCount = m_swapchain->ImageCount();
	}
}