#include "RenderSystem.h"

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandPool.h>
#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Instance/GfxInstance.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>
#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/Graphics/RHI/Sync/GfxFence.h>
#include <Runtime/Graphics/RHI/Sync/GfxSemaphore.h>

#include <Application/Systems/Window/WindowSystem.h>

namespace Horizon
{
	static constexpr u8 MaxFrameInFlight = 2;

    EngineReport RenderSystem::OnInitialize()
	{
		m_instance = GfxInstance::Create(GfxInstanceDesc()
			.setAPIType(GfxType::Vulkan)
			.setAppName("Test")
			.setAppVersion({ 1, 0, 0 }));
		if (!m_instance)
			return EngineReport("RHI", "failed initializing GfxInstance");

		m_device = m_instance->CreateDevice(GfxDeviceDesc()
			.setComputeQueueCount(1)
			.setGraphicsQueueCount(1)
			.setTransferQueueCount(1));
		if(!m_device)
			return EngineReport("RHI", "failed initializing GfxDevice");

		m_graphicsQueue = m_device->CreateQueue(QueueType::Graphics);
		m_computeQueue = m_device->CreateQueue(QueueType::Compute);
		m_transferQueue = m_device->CreateQueue(QueueType::Transfer);

		auto& windowSys = RequestSystem<WindowSystem>();

		m_swapchain = m_device->CreateSwapchain(GfxSwapchainDesc()
			.setFormat(ImageFormat::R8G8B8A8_UNorm)
			.setGraphicsQueue(m_graphicsQueue.get())
			.setImageCount(3)
			.setImageSize(windowSys.WindowSize())
			.setPresentMode(PresentMode::Mailbox)
			.setWindowAPI(windowSys.APIWindow())
			.setWindowHandler(windowSys.Handle())
			.setWindowInstance(windowSys.Instance()));
		if (!m_swapchain)
			return EngineReport("RHI", "failed initializing GfxSwapchain");

		m_cmdPool = m_device->CreateCommandPool(GfxCommandPoolDesc()
			.setQueue(m_graphicsQueue.get())
			.setFlags(CommandPoolFlags::ResetCommandBuffer));

		for (size_t i = 0; i < MaxFrameInFlight; i++)
			m_cmdBuffers.push_back(m_device->CreateCommandBuffer(GfxCommandBufferDesc()
				.setLevel(CommandLevel::Primary)
				.setPool(m_cmdPool.get())));

		for (size_t i = 0; i < MaxFrameInFlight; i++)
			m_fences.push_back(m_device->CreateSyncFence(GfxFenceDesc().setSignal(true)));

		for (size_t i = 0; i < MaxFrameInFlight; i++)
			m_imageAvailableSemaphores.push_back(m_device->CreateSyncSemaphore(GfxSemaphoreDesc()));

		for (size_t i = 0; i < m_swapchain->ImageCount(); i++)
			m_renderFinishedSemaphores.push_back(m_device->CreateSyncSemaphore(GfxSemaphoreDesc()));

		return EngineReport();
	}

	void RenderSystem::OnSync()
	{
		m_fences[m_frameIndex]->WaitIdle();
		m_fences[m_frameIndex]->Reset();

		u32 imageIndex = m_swapchain->AcquireNextImage(m_imageAvailableSemaphores[m_frameIndex].get(), nullptr);
		auto* swapImage = m_swapchain->Image(imageIndex);

		m_cmdBuffers[m_frameIndex]->BeginRecord();

		m_cmdBuffers[m_frameIndex]->ImageBarrier(ImageBarrierDesc()
			.setImage(swapImage)
			.setOldLayout(ImageLayout::Undefined)
			.setNewLayout(ImageLayout::ColorAttachmentOptimal)
			.setAspect(ImageAspect::Color));

		m_cmdBuffers[m_frameIndex]->BeginRendering(RenderingInfo()
			.setRenderAreaExtent(m_swapchain->ImageSize())
			.addColorAttachment(RenderingAttachmentInfo()
				.setImageView(m_swapchain->ImageView(imageIndex))
				.setImageLayout(ImageLayout::ColorAttachmentOptimal)
				.setLoadOp(AttachmentLoad::Clear)
				.setStoreOp(AttachmentStore::Store)
				.setClearValue(ClearValue().setColor({ 1.f, 1.f, 0.f, 1.f })))
			.setDepthAttachment(nullptr)
			.setFlags(RenderingFlags::ContentsSecondary));

		m_cmdBuffers[m_frameIndex]->ExecuteCommands(m_renderBatch.Flush(m_frameIndex));

		m_cmdBuffers[m_frameIndex]->EndRendering();
		m_cmdBuffers[m_frameIndex]->ImageBarrier(ImageBarrierDesc()
			.setImage(swapImage)
			.setOldLayout(ImageLayout::ColorAttachmentOptimal)
			.setNewLayout(ImageLayout::PresentSrcKHR)
			.setAspect(ImageAspect::Color));
		m_cmdBuffers[m_frameIndex]->EndRecord();

		m_graphicsQueue->Submit(
			{ m_cmdBuffers[m_frameIndex].get() },
			{ m_imageAvailableSemaphores[m_frameIndex].get() },
			{ m_renderFinishedSemaphores[imageIndex].get() },
			m_fences[m_frameIndex].get(),
			PipelineStageFlags::ColorAttachment);

		m_swapchain->Present({ m_renderFinishedSemaphores[imageIndex].get() });

		m_frameIndex = (m_frameIndex + 1) % MaxFrameInFlight;
	}

	void RenderSystem::OnFinalize()
	{
		m_device->WaitIdle();
	}
}