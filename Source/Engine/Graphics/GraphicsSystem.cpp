#include "GraphicsSystem.h"

#include <Runtime/Graphics/RHI/Command/GfxCommandBuffer.h>
#include <Runtime/Graphics/RHI/Command/GfxCommandPool.h>
#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Instance/GfxInstance.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>
#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/Graphics/RHI/Framebuffer/GfxFramebuffer.h>
#include <Runtime/Graphics/RHI/RenderPass/GfxRenderPass.h>
#include <Runtime/Graphics/RHI/Sync/GfxFence.h>
#include <Runtime/Graphics/RHI/Sync/GfxSemaphore.h>

#include <Engine/Window/WindowSystem.h>
#include <Engine/Presentation/PresentationSystem.h>
#include <Engine/Renderer/EditorRenderer.h>

#include <Runtime/Graphics/RHI/Buffer/GfxBuffer.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorLayout.h>
#include <Runtime/Graphics/RHI/Descriptor/GfxDescriptorBuffer.h>

#include <Runtime/ShaderCompiler/ShaderCompiler.h>

namespace Horizon
{
	GraphicsSystem::GraphicsSystem() : m_currentFrame(0), m_currentImageIndex(0)
	{
	}

	SystemReport GraphicsSystem::OnInitialize()
	{
		auto& windowSystem = RequestSystem<WindowSystem>();

		m_instance = GfxInstance::Create(
			GfxInstanceDesc()
			.setAppName("Horizon")
			.setAppVersion({ 1, 0, 0 })
			.setAPIType(GfxType::Vulkan)
		);
		m_device = m_instance->CreateDevice(
			GfxDeviceDesc()
			.setGraphicsQueueCount(1)
			.setComputeQueueCount(1)
			.setTransferQueueCount(0)
		);
		m_graphicsQueue = m_device->CreateQueue(QueueType::Graphics);
		m_computeQueue = m_device->CreateQueue(QueueType::Compute);

		m_swapchain = m_device->CreateSwapchain(
			GfxSwapchainDesc()
			.setImageSize(windowSystem.GetWindowSize())
			.setImageCount(2)
			.setFormat(ImageFormat::R8G8B8A8_UNorm)
			.setPresentMode(PresentMode::Fifo)
			.setGraphicsQueue(m_graphicsQueue.get())
			.setWindowHandler(windowSystem.GetWindowHandle())
		);

		m_graphicsCmdPool = m_device->CreateCommandPool(
			GfxCommandPoolDesc()
			.setQueue(m_graphicsQueue.get())
			.setFlags(CommandPoolFlags::ResetCommandBuffer)
		);

		for (usize it = 0; it < m_swapchain->ImageCount(); it++)
		{
			m_graphicsCmds.push_back(m_graphicsCmdPool->CreateBuffer(CommandLevel::Primary));
			m_frameFences.push_back(m_device->CreateSyncFence(GfxFenceDesc().setSignal(false)));
			m_sceneFinishedSems.push_back(m_device->CreateSyncSemaphore(GfxSemaphoreDesc()));
		}

		m_presentationPass = m_device->CreateRenderPass(
			GfxRenderPassDesc()
			.addAttachment(
				AttachmentDesc()
				.setFormat(m_swapchain->ImageFormat())
				.setType(AttachmentType::Color)
				.setInitialLayout(ImageLayout::Undefined)
				.setFinalLayout(ImageLayout::PresentSrcKHR)
				.setLoadOp(AttachmentLoad::Clear)
				.setStoreOp(AttachmentStore::Store)
			)
			.addSubpass(
				SubpassDesc()
				.addColorAttachment(0)
				.setBindPoint(PipelineType::Graphics)
			)
		);

		const Math::Vec2u& windowSize = m_swapchain->ImageSize();
		GfxFramebufferDesc fbDesc;
		fbDesc.setPassRef(m_presentationPass.get())
			.setFramebufferSize({ windowSize.x, windowSize.y, 1 });

		for (usize i = 0; i < m_swapchain->ImageCount(); i++)
			fbDesc.addFramebufferViews({ m_swapchain->ImageView(i) });
		m_presentationFramebuffer = m_device->CreateFramebuffer(fbDesc);

		// Initialize the editor renderer.
		m_editor = std::make_shared<EditorRenderer>();
		m_editor->SetEngine(GetEngine());
		if (!m_editor->Initialize(this))
			return SystemReport(GetObjectType(), "Failed to initialize editor renderer.");

		return SystemReport();
	}

	void GraphicsSystem::OnSync()
	{
		m_currentImageIndex = m_swapchain->AcquireNextImage(nullptr, m_frameFences[m_currentFrame].get());
		m_frameFences[m_currentFrame]->WaitIdle();
		m_frameFences[m_currentFrame]->Reset();

		m_graphicsCmds[m_currentFrame]->BeginRecord(CommandBufferUsage::OneTimeSubmit);
		m_graphicsCmds[m_currentFrame]->BeginRenderPass(BeginRenderDesc()
			.setRenderPass(m_presentationPass.get())
			.setFramebuffer(m_presentationFramebuffer.get())
			.setFrameIndex(m_currentImageIndex)
			.setRenderArea(m_swapchain->ImageSize())
			.setColor({ .1f, .2f, .3f, 1.f })
			.setClearColor(true)
		);

		for (auto& request : m_renderRequests)
			request.command(m_graphicsCmds[m_currentFrame].get());
		m_renderRequests.clear();

		m_editor->Render(m_graphicsCmds[m_currentFrame].get());

		m_graphicsCmds[m_currentFrame]->EndRenderPass();
		m_graphicsCmds[m_currentFrame]->EndRecord();

		auto& presentSys = RequestSystem<PresentationSystem>();
		presentSys.RequestPresent(PresentRequest()
			.setCmdBuffer(m_graphicsCmds[m_currentFrame].get())
			.setImageIndex(m_currentImageIndex)
		);
	}

	void GraphicsSystem::OnFinalize()
	{
		for(usize i = 0; i < m_swapchain->ImageCount(); i++)
		{
			m_frameFences[i]->WaitIdle();
			m_sceneFinishedSems[i]->WaitIdle();
		}

		m_device->WaitIdle();
	}

	void GraphicsSystem::AdvanceFrame()
	{
		m_currentFrame = (m_currentFrame + 1) % m_swapchain->ImageCount();
	}
}
