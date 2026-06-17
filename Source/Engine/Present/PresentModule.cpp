#include "PresentModule.h"

#include <Engine/Engine/Engine.h>
#include <Engine/Graphics/GraphicsModule.h>
#include <Engine/Window/WindowModule.h>

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>

namespace Horizon
{
	void PresentModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);

		auto& windowModule = engine.GetModule<WindowModule>();
		const Window& window = windowModule.GetMainWindow();

		auto& graphicsModule = engine.GetModule<GraphicsModule>();
		GfxDevice* device = graphicsModule.GetDevice();
		GfxQueue* gQueue = graphicsModule.GetGraphicsQueue();

		GfxSwapchainDesc swapchainDesc = {};
		swapchainDesc.pWindowHandle = window.GetOSHandle();
		swapchainDesc.width = window.GetSize().x;
		swapchainDesc.height = window.GetSize().y;
		swapchainDesc.imageCount = 3;
		swapchainDesc.vSync = true;

		m_swapchain = device->CreateSwapchain(swapchainDesc, gQueue);
	}

	void PresentModule::OnSync()
	{
		// First try to present
		//m_swapchain->Present();

		// Then acquire next image
	}

	void PresentModule::OnDetach()
	{}
}