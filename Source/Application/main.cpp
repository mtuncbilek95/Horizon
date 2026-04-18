#include <Runtime/Window/Window.h>

#include <Runtime/Graphics/RHI/Instance/GfxInstance.h>
#include <Runtime/Graphics/RHI/Device/GfxDevice.h>

using namespace Horizon;

int main(int argc, char* argv[])
{
	auto window = Window(WindowDesc()
		.setWindowMode(WindowMode::Windowed)
		.setWindowName("Test")
		.setWindowSize({ 1920u, 1080u }));
	window.Show();

	auto instance = GfxInstance::Create(GfxInstanceDesc()
		.setAPIType(GfxType::Vulkan)
		.setAppName("Test")
		.setAppVersion({ 1, 0, 0 }));

	auto device = instance->CreateDevice(GfxDeviceDesc()
		.setComputeQueueCount(1)
		.setGraphicsQueueCount(1)
		.setTransferQueueCount(1));
	auto gQueue = device->CreateQueue(QueueType::Graphics);
	auto tQueue = device->CreateQueue(QueueType::Transfer);

	auto swapchain = device->CreateSwapchain(GfxSwapchainDesc()
		.setFormat(ImageFormat::R8G8B8A8_UNorm)
		.setGraphicsQueue(gQueue.get())
		.setImageCount(2)
		.setImageSize(window.GetSize())
		.setPresentMode(PresentMode::Mailbox)
		.setWindowHandler(window.GetHandle())
		.setWindowInstance(window.GetInstance())
		.setWindowAPI(window.GetNativeWindow()));

	while (window.IsActive())
	{
		window.ProcessEvents();
	}

}