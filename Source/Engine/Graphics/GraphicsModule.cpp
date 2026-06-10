#include "GraphicsModule.h"

#include <Engine/Engine/Engine.h>
#include <Engine/Window/WindowModule.h>

namespace Horizon
{
	GraphicsModule::GraphicsModule()
	{}

	GraphicsModule::~GraphicsModule()
	{}

	void GraphicsModule::OnAttach(Engine& engine)
	{
		IModule::OnAttach(engine);

		auto* pWinModule = engine.TryGetModule<WindowModule>();

		m_mainDevice = Gfx::CreateGfxDevice(GfxDeviceDesc()
			.setDebug(true)
			.setGPUValidation(false));

		auto& currWindow = pWinModule->GetMainWindow();
		m_swapchain = Gfx::CreateGfxSwapchain(m_mainDevice, GfxSwapchainDesc()
			.setSync(true)
			.setWindowHandle(currWindow.GetOSHandle())
			.setImgSize(currWindow.GetSize().x, currWindow.GetSize().y)
			.setImgCount(3));
	}

	void GraphicsModule::OnDetach()
	{
		Gfx::ShutdownGfxDevice(m_mainDevice);
	}
}