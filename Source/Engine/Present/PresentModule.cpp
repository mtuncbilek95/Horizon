#include "PresentModule.h"

#include <Engine/Engine/Engine.h>
#include <Engine/Graphics/GraphicsModule.h>
#include <Engine/Graphics/FrameContext.h>
#include <Engine/Window/WindowModule.h>

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>

namespace Horizon
{
	void PresentModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);

		auto& windowModule = engine.GetModule<WindowModule>();
		const Window& window = windowModule.GetMainWindow();

		auto& graphicsModule = engine.GetModule<GraphicsModule>();
		m_device = graphicsModule.GetDevice();
		m_graphicsQueue = graphicsModule.GetGraphicsQueue();
		m_frameContext = &graphicsModule.GetFrameContext();

		m_width = window.GetSize().x;
		m_height = window.GetSize().y;

		GfxSwapchainDesc swapchainDesc = {};
		swapchainDesc.pWindowHandle = window.GetOSHandle();
		swapchainDesc.width = m_width;
		swapchainDesc.height = m_height;
		swapchainDesc.imageCount = 3;
		swapchainDesc.vSync = true;

		m_swapchain = m_device->CreateSwapchain(swapchainDesc, m_graphicsQueue);
		m_uiCommandList = m_device->CreateCommandList(GfxQueueType::Graphics);
	}

	void PresentModule::OnSync()
	{
		const u32 index = m_swapchain->GetCurrentIndex();
		GfxTexture* backbuffer = m_swapchain->GetBackbuffer(index);

		GfxCommandList* cmd = m_uiCommandList.get();
		cmd->Begin();
		cmd->SetupBindless();

		GfxTextureBarrier toRender = { backbuffer, GfxResourceState::Present, GfxResourceState::RenderTarget };
		cmd->Barrier(&toRender, 1);

		GfxRenderBeginDesc uiPass = {};
		uiPass.addColorTarget(backbuffer, GfxLoadOp::Clear, { 0.0f, 0.0f, 0.0f, 1.0f })
			.setSize(m_width, m_height);
		cmd->BeginRendering(uiPass);

		cmd->DrawImGui();

		GfxTextureBarrier toPresent = { backbuffer, GfxResourceState::RenderTarget, GfxResourceState::Present };
		cmd->Barrier(&toPresent, 1);

		cmd->End();

		GfxCommandList* lists[] = { cmd };
		m_graphicsQueue->Submit(lists, 1);

		m_swapchain->Present();

		m_frameContext->EndFrame();
	}

	void PresentModule::OnDetach()
	{
		m_frameContext->WaitIdle();

		m_uiCommandList.reset();
		m_swapchain.reset();
	}
}
