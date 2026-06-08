#include <Runtime/Window/Window.h>
#include <Runtime/Graphics/RHI/GfxContext.h>

using namespace Horizon;

int main()
{
	Window window = Window(WindowDesc());

	GfxDevice::InitializeDevice(GfxDeviceDesc());

	GfxSwapchainDesc scDesc = {};
	scDesc.windowHandle = window.GetOSHandle();
	scDesc.width = window.GetSize().x;
	scDesc.height = window.GetSize().y;
	scDesc.vsync = true;
	GfxDevice::CreateSwapchain(scDesc);

	window.Show();

	const u32 width = window.GetSize().x;
	const u32 height = window.GetSize().y;

	GfxFenceHandle frameFence = GfxDevice::CreateFence(GfxFenceDesc());
	u64 slotTarget[MaxFramesInFlight] = {};
	u64 frameIndex = 0;

	while (window.IsActive())
	{
		window.PollEvents();

		u32 slot = u32(frameIndex % MaxFramesInFlight);
		GfxFence::WaitForCPU(frameFence, slotTarget[slot]);
		GfxDevice::ResetCommandPools(slot);

		GfxTextureHandle backbuffer = GfxSwapchain::AcquireNext();
		GfxCmdListHandle commandList = GfxDevice::CreateCommandList({ GfxQueueType::Graphics, 0 });

		GfxTextureBarrier toRenderTarget = { backbuffer, GfxResourceState::Present, GfxResourceState::RenderTarget };
		GfxCmdList::Barrier(commandList, { &toRenderTarget, 1 });

		GfxColorAttachment colorAttachment = {};
		colorAttachment.texture = backbuffer;
		colorAttachment.loadOp = GfxLoadOp::Clear;
		colorAttachment.clearColor = { 0.1f, 0.2f, 0.4f, 1.0f };

		GfxRenderBeginDesc renderBegin = {};
		renderBegin.colorTargets = { &colorAttachment, 1 };
		renderBegin.width = width;
		renderBegin.height = height;
		GfxCmdList::BeginRendering(commandList, renderBegin);

		GfxTextureBarrier toPresent = { backbuffer, GfxResourceState::RenderTarget, GfxResourceState::Present };
		GfxCmdList::Barrier(commandList, { &toPresent, 1 });

		GfxCmdList::EndRendering(commandList);

		GfxCmdListHandle lists[] = { commandList };
		GfxQueue::ExecuteCommandLists(GfxQueueType::Graphics, lists);

		u64 value = GfxFence::ReserveValue(frameFence);
		GfxQueue::Signal(GfxQueueType::Graphics, frameFence, value);
		slotTarget[slot] = value;

		GfxSwapchain::Present();
		++frameIndex;
	}

	for (u32 i = 0; i < MaxFramesInFlight; i++)
		GfxFence::WaitForCPU(frameFence, slotTarget[i]);

	GfxDevice::ShutdownDevice();

	return 0;
}
