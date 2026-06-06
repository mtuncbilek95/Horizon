#include "Renderer.h"
#include "SceneLoader.h"
#include "CameraController.h"

#include <Engine/Window/Window.h>
#include <Engine/Window/InputDispatcher.h>

#include <Engine/Graphics/RHI/GfxDevice.h>
#include <Engine/Graphics/RHI/GfxSwapchain.h>

#include <Engine/FrameGraph/FrameGraph.h>

#include <Engine/ECS/ECS.h>
#include <Engine/ECS/Components/CameraComp.h>

using namespace Horizon;

int main()
{
	MainWindow();

	GfxDeviceDesc deviceDesc = {};
	deviceDesc.windowHandle = MainWindow().GetOSHandle();
	deviceDesc.framesInFlight = 2;
	deviceDesc.maxWorkers = 1;
#if defined(_DEBUG)
	deviceDesc.enableDebug = true;
#endif
	GfxDevice::Initialize(deviceDesc);

	const u32 framesInFlight = deviceDesc.framesInFlight;

	GfxSwapchainDesc swapchainDesc = {};
	swapchainDesc.windowHandle = MainWindow().GetOSHandle();
	swapchainDesc.width = MainWindow().GetSize().x;
	swapchainDesc.height = MainWindow().GetSize().y;
	swapchainDesc.imageCount = 2;
	swapchainDesc.format = GfxTextureFormat::RGBA8;
	swapchainDesc.vsync = true;
	GfxSwapchain::Create(swapchainDesc);

	InputSystem().OnResizeWindow([](const InputMessage& msg)
		{
			if (msg.resizeWidth == 0 || msg.resizeHeight == 0)
				return;
			GfxDevice::WaitIdle();
			GfxSwapchain::Resize(msg.resizeWidth, msg.resizeHeight);
		});

	EntityHandle camera = LoadScene("Cooked/Sponza/Sponza.hmodel");
	InstallCameraControls(camera);

	Renderer renderer;
	renderer.Initialize(framesInFlight);

	u64 frameCounter = 0;
	while (MainWindow().IsActive())
	{
		MainWindow().PollEvents();

		const u32 windowWidth = MainWindow().GetSize().x;
		const u32 windowHeight = MainWindow().GetSize().y;
		const u32 frameSlot = (u32)(frameCounter % framesInFlight);

		CameraComp& cam = MainWorld().GetComponent<CameraComp>(camera);
		cam.aspectRatio = (f32)windowWidth / (f32)windowHeight;

		GfxDevice::BeginFrame();

		GfxTextureHandle backBuffer = GfxSwapchain::AcquireNext();
		renderer.Render(cam, backBuffer, windowWidth, windowHeight, frameSlot);

		GfxSwapchain::Present();
		GfxDevice::EndFrame();

		frameCounter++;
	}

	GfxDevice::WaitIdle();
	renderer.Shutdown();
	MainGraph().Shutdown();
	GfxDevice::Shutdown();
}