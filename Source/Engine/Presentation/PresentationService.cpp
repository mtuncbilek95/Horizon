#include "PresentationService.h"

#include <Engine/Core/Engine.h>
#include <Engine/Core/ModuleDependency.h>
#include <Engine/Graphics/GraphicsContext.h>
#include <Engine/Window/WindowService.h>
#include <Engine/World/WorldService.h>

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
#include <Runtime/PAL/Window/Window.h>
#include <Runtime/RHI/Command/RenderingDesc.h>
#include <Runtime/RHI/Device/Device.h>
#include <Runtime/RHI/Texture/TextureDesc.h>
#include <Runtime/RHI/Texture/TextureViewDesc.h>

namespace Horizon::Engine
{
	PresentationService::PresentationService(PresentationMode mode) : m_mode(mode)
	{
	}

	PresentationService::~PresentationService() = default;

	ModuleReport PresentationService::OnInitialize()
	{
		// Get window service to use on swapchain
		auto* pWindowSub = GetEngine()->RequestService<WindowService>();
		if (!pWindowSub)
			return ModuleReport("Failed to get WindowService. Nothing will be presented...");

		auto* pGraphSub = GetEngine()->RequestContext<GraphicsContext>();
		if (!pGraphSub)
			return ModuleReport("Failed to get GraphicsContext. Nothing will be presented...");

		m_window = pWindowSub->GetWindow();
		m_device = pGraphSub->GetDevice();
		m_resourceHeap = pGraphSub->GetResourceHeap();
		m_samplerHeap = pGraphSub->GetSamplerHeap();

		PAL::WindowRect windowRect = m_window->GetRect();

		// Create swapchain
		RHI::SwapchainDesc swapDesc = {};
		swapDesc.windowHandle = m_window->GetOSHandle();
		swapDesc.imageCount = 3;
		swapDesc.width = windowRect.width;
		swapDesc.height = windowRect.height;
		swapDesc.presentMode = RHI::PresentMode::Immediate;
		swapDesc.format = RHI::TextureFormat::RGBA8_UNORM;
		m_swapchain = m_device->CreateRHISwapchain(swapDesc);
		if (!m_swapchain)
			return ModuleReport("Failed to create Swapchain");

		m_frames.Resize(GraphicsContext::MaxFramesInFlight);
		for (u32 i = 0; i < GraphicsContext::MaxFramesInFlight; i++)
		{
			RHI::CommandPoolDesc poolDesc = {};
			poolDesc.type = RHI::QueueType::Graphics;
			m_frames[i].pPool = m_device->CreateRHICommandPool(poolDesc);
			if (!m_frames[i].pPool)
				return ModuleReport("Failed to create frame CommandPool");

			RHI::CommandListDesc listDesc = {};
			listDesc.pPool = m_frames[i].pPool.Get();
			m_frames[i].pCmdList = m_device->CreateRHICommandList(listDesc);
			if (!m_frames[i].pCmdList)
				return ModuleReport("Failed to create frame CommandList");
		}

		m_fence = m_device->CreateRHIFence({});
		if (!m_fence)
			return ModuleReport("Failed to create frame Fence");

		if (!CreateSceneTarget(Math::Vec2u(windowRect.width, windowRect.height)))
			return ModuleReport("Failed to create scene target");

		if (m_mode == PresentationMode::Game)
			Terminal::Warn(StringHelper::GetTypeName(this), "Game mode has no blit pass yet, backbuffer will only be cleared");

		Terminal::Debug(StringHelper::GetTypeName(this), "Swapchain, frame slots and scene target has been initialized!");

		return ModuleReport();
	}

	void PresentationService::OnExecute(const EngineFrame& ctx)
	{
		HandleWindowMessages();

		const b8 acquired = !m_minimized && m_swapchain->AcquireNextImage();

		if (acquired)
		{
			FrameSlot& slot = m_frames[m_frameIndex];

			m_fence->WaitCPU(slot.fenceValue);
			slot.pPool->Reset();

			RHI::CommandList* pCmdList = slot.pCmdList.Get();
			Record(pCmdList);

			m_device->Submit(RHI::QueueType::Graphics, { pCmdList });

			slot.fenceValue = m_device->Signal(RHI::QueueType::Graphics, m_fence.Get());
			m_frameIndex = (m_frameIndex + 1) % GraphicsContext::MaxFramesInFlight;
		}

		if (m_overlay)
			m_overlay->OnFrameEnd();

		if (acquired)
			m_swapchain->Present();

		ApplySceneExtent();
	}

	void PresentationService::OnFinalize()
	{
		if (m_device)
			m_device->WaitIdle();

		m_overlay = nullptr;

		m_sceneView = nullptr;
		m_sceneTexture = nullptr;

		m_frames.Clear();
		m_fence = nullptr;

		m_swapchain = nullptr;
	}

	void PresentationService::DeclareDependencies(ModuleDependency& graph)
	{
		graph.InitAfter<WindowService, GraphicsContext>();
		graph.InitBefore<WorldService>();

		graph.RunAfter<WorldService>();
	}

	void PresentationService::HandleWindowMessages()
	{
		const auto& messages = m_window->GetMessages();

		b8 resized = false;
		u32 width = 0;
		u32 height = 0;

		for (const auto& message : messages)
		{
			if (message.type != PAL::InputMessageType::Resize)
				continue;

			width = message.width;
			height = message.height;
			resized = true;
		}

		if (!resized)
			return;

		m_minimized = width == 0 || height == 0;
		if (m_minimized)
			return;

		m_device->WaitIdle();
		m_swapchain->Resize(width, height);

		if (m_mode == PresentationMode::Game)
			m_pendingExtent = Math::Vec2u(width, height);
	}

	void PresentationService::Record(RHI::CommandList* pCmdList)
	{
		RHI::Texture* pBackbuffer = m_swapchain->GetCurrentImage();

		pCmdList->Start();
		pCmdList->BindDescriptorHeaps(m_resourceHeap, m_samplerHeap);

		pCmdList->Barrier({ RHI::TextureBarrier{ pBackbuffer, RHI::ResourceState::Undefined, RHI::ResourceState::RenderTarget } });

		RHI::RenderingDesc rendering = {};
		rendering.colorAttachments[0].pView = m_swapchain->GetCurrentImageView();
		rendering.colorAttachments[0].loadOp = RHI::LoadOp::Clear;
		rendering.colorAttachments[0].clearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		rendering.colorAttachmentCount = 1;

		pCmdList->StartRendering(rendering);

		if (m_overlay)
			m_overlay->OnRecord(pCmdList);

		pCmdList->EndRendering();

		pCmdList->Barrier({ RHI::TextureBarrier{ pBackbuffer, RHI::ResourceState::RenderTarget, RHI::ResourceState::Present } });

		pCmdList->End();
	}

	b8 PresentationService::CreateSceneTarget(Math::Vec2u extent)
	{
		RHI::TextureDesc textureDesc = {};
		textureDesc.format = SceneFormat;
		textureDesc.usage = RHI::TextureUsage::RenderTarget | RHI::TextureUsage::Sampled | RHI::TextureUsage::Storage;
		textureDesc.width = extent.X();
		textureDesc.height = extent.Y();

		UniquePtr<RHI::Texture> pTexture = m_device->CreateRHITexture(textureDesc);
		if (!pTexture)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Scene texture could not be created at {}x{}", extent.X(), extent.Y());
			return false;
		}

		RHI::TextureViewDesc viewDesc = {};
		viewDesc.pTexture = pTexture.Get();
		viewDesc.format = SceneFormat;

		UniquePtr<RHI::TextureView> pView = m_device->CreateRHITextureView(viewDesc);
		if (!pView)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Scene texture view could not be created at {}x{}", extent.X(), extent.Y());
			return false;
		}

		FrameSlot& slot = m_frames[m_frameIndex];
		slot.pPool->Reset();

		RHI::CommandList* pCmdList = slot.pCmdList.Get();
		pCmdList->Start();

		pCmdList->Barrier({ RHI::TextureBarrier{ pTexture.Get(), RHI::ResourceState::Undefined, RHI::ResourceState::RenderTarget } });

		RHI::RenderingDesc rendering = {};
		rendering.colorAttachments[0].pView = pView.Get();
		rendering.colorAttachments[0].loadOp = RHI::LoadOp::Clear;
		rendering.colorAttachments[0].clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		rendering.colorAttachmentCount = 1;

		pCmdList->StartRendering(rendering);
		pCmdList->EndRendering();

		pCmdList->Barrier({ RHI::TextureBarrier{ pTexture.Get(), RHI::ResourceState::RenderTarget, RHI::ResourceState::ShaderResource } });

		pCmdList->End();
		m_device->Submit(RHI::QueueType::Graphics, { pCmdList });

		slot.fenceValue = m_device->Signal(RHI::QueueType::Graphics, m_fence.Get());

		m_sceneView = nullptr;
		m_sceneTexture = std::move(pTexture);
		m_sceneView = std::move(pView);

		m_sceneExtent = extent;
		m_pendingExtent = extent;

		return true;
	}

	void PresentationService::ApplySceneExtent()
	{
		if (m_pendingExtent.X() == 0 || m_pendingExtent.Y() == 0)
			return;

		if (m_pendingExtent == m_sceneExtent)
			return;

		m_device->WaitIdle();

		if (!CreateSceneTarget(m_pendingExtent))
			m_pendingExtent = m_sceneExtent;
	}
}