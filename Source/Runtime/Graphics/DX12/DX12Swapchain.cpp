#include "DX12Context.h"

#include <Runtime/Graphics/DX12/DX12Context.h>

namespace Horizon
{
	void GfxDevice::CreateSwapchain(const GfxSwapchainDesc& desc)
	{
		Context& context = DX12Context();
		context.bSyncPresent = desc.vsync;
		context.imageCount = desc.imageCount;

		DXGI_SWAP_CHAIN_DESC1 scDesc = {};
		scDesc.Width = desc.width;
		scDesc.Height = desc.height;
		scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		scDesc.SampleDesc = { 1, 0 };
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.BufferCount = desc.imageCount;
		scDesc.Scaling = DXGI_SCALING_STRETCH;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

		IDXGISwapChain1* swapchain1 = nullptr;

		HRESULT hresult = context.pFactory->CreateSwapChainForHwnd(context.queuePool[u32(GfxQueueType::Graphics)], 
			(HWND)desc.windowHandle, &scDesc, nullptr, nullptr, &swapchain1);
		CHECK_HR(hresult, "IDXGISwapChain1 - CreateSwapChainForHwnd");

		hresult = swapchain1->QueryInterface(IID_PPV_ARGS(&context.pSwapchain));
		CHECK_HR(hresult, "IDXGISwapChain4 - QueryInterface");

		swapchain1->Release();

		Helpers::CreateBackbuffers(desc.width, desc.height);
	}

	GfxTextureHandle GfxSwapchain::AcquireNext()
	{
		Context& context = DX12Context();
		u32 index = context.pSwapchain->GetCurrentBackBufferIndex();
		return context.backBufferPool[index];
	}

	void GfxSwapchain::Resize(u32 width, u32 height)
	{
		Context& context = DX12Context();

		GfxDevice::WaitIdle();

		// Clear out Backbuffers

		// Add Create Backbuffers
	}

	void GfxSwapchain::Present()
	{
		Context& context = DX12Context();
		HRESULT bResult = context.pSwapchain->Present(context.bSyncPresent ? 1 : 0, 0);
		CHECK_REASON(bResult, "IDXGISwapChain4 - Present");
	}
}