#include "DX12Backend.h"

#include <Engine/Log/Log.h>
#include <Engine/Graphics/RHI/GfxSwapchain.h>

#define CHECK_HR(hr, what) \
	if (FAILED(hr)) { ConsoleLog().Error("{}: {}", what, _com_error(hr).ErrorMessage()); exit(-1); }

namespace Horizon
{
	using namespace DX12;

	static void CreateBackbuffers(u32 width, u32 height)
	{
		Context& context = GfxContext();
		for (u32 backbufferIdx = 0; backbufferIdx < context.backbuffers.size(); backbufferIdx++)
		{
			ID3D12Resource* resource = nullptr;
			HRESULT hresult = context.swapchain->GetBuffer(backbufferIdx, IID_PPV_ARGS(&resource));
			CHECK_HR(hresult, "GetBuffer");

			GfxTextureHandle handle = TexturePoolAlloc();
			DX12Texture& texture = TexturePoolGet(handle);
			texture.resource = resource;
			texture.memory = nullptr;
			texture.state = D3D12_RESOURCE_STATE_PRESENT;
			texture.format = context.swapchainFormat;
			texture.width = width; texture.height = height;
			texture.isBackbuffer = true;
			texture.targetViewIndex = context.nextTargetView++;

			D3D12_CPU_DESCRIPTOR_HANDLE targetView = context.targetViewHeap->GetCPUDescriptorHandleForHeapStart();
			targetView.ptr += usize(texture.targetViewIndex) * context.targetViewStride;

			D3D12_RENDER_TARGET_VIEW_DESC targetViewDesc = {};
			targetViewDesc.Format = context.swapchainFormat;
			targetViewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			context.device->CreateRenderTargetView(resource, &targetViewDesc, targetView);

			SetDebugName(resource, "Swapchain Texture");

			context.backbuffers[backbufferIdx] = handle;
		}

		context.backbufferIndex = context.swapchain->GetCurrentBackBufferIndex();
	}

	void GfxSwapchain::Create(const GfxSwapchainDesc& desc)
	{
		Context& context = GfxContext();
		context.swapchainFormat = ToDXGI(desc.format);
		context.backbuffers.resize(desc.imageCount);
		context.vsync = desc.vsync;

		DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};
		swapchainDesc.Width = desc.width;
		swapchainDesc.Height = desc.height;
		swapchainDesc.Format = context.swapchainFormat;
		swapchainDesc.SampleDesc = { 1, 0 };
		swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDesc.BufferCount = desc.imageCount;
		swapchainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

		IDXGISwapChain1* swapchain1 = nullptr;

		HRESULT hresult = context.factory->CreateSwapChainForHwnd(context.queues[u32(GfxQueueType::Graphics)], (HWND)desc.windowHandle, &swapchainDesc, nullptr, nullptr, &swapchain1);
		CHECK_HR(hresult, "CreateSwapChainForHwnd");

		hresult = swapchain1->QueryInterface(IID_PPV_ARGS(&context.swapchain));
		CHECK_HR(hresult, "QueryInterface SwapChain4");

		swapchain1->Release();

		CreateBackbuffers(desc.width, desc.height);
	}

	GfxTextureHandle GfxSwapchain::AcquireNext()
	{
		Context& context = GfxContext();
		context.backbufferIndex = context.swapchain->GetCurrentBackBufferIndex();
		return context.backbuffers[context.backbufferIndex];
	}

	void GfxSwapchain::Present()
	{
		Context& context = GfxContext();
		HRESULT hresult = context.swapchain->Present(context.vsync ? 1 : 0, 0);
		CHECK_HR(hresult, "Present");
	}

	void GfxSwapchain::Resize(u32 width, u32 height)
	{
		Context& context = GfxContext();
		GfxDevice::WaitIdle();

		for (u32 backbufferIdx = 0; backbufferIdx < context.backbuffers.size(); backbufferIdx++)
		{
			TexturePoolGet(context.backbuffers[backbufferIdx]).resource->Release();
			TexturePoolFree(context.backbuffers[backbufferIdx]);
		}
		context.nextTargetView = 0;

		HRESULT hresult = context.swapchain->ResizeBuffers(context.backbuffers.size(), width, height, context.swapchainFormat, 0);
		CHECK_HR(hresult, "ResizeBuffers");
		CreateBackbuffers(width, height);
	}

	void GfxSwapchain::Destroy()
	{
		Context& context = GfxContext();
		for (u32 backbufferIdx = 0; backbufferIdx < context.backbuffers.size(); backbufferIdx++)
		{
			if (context.backbuffers[backbufferIdx].isValid())
			{
				TexturePoolGet(context.backbuffers[backbufferIdx]).resource->Release();
				TexturePoolFree(context.backbuffers[backbufferIdx]);
			}
		}

		if (context.swapchain)
			context.swapchain->Release();
	}
}
