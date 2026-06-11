#include "D3D12Backend.h"

#include <Runtime/Graphics/GfxBackend.h>

namespace Horizon
{
	GfxSwapchain* Gfx::CreateGfxSwapchain(GfxDevice* pContext, GfxQueue* pQueue, const GfxSwapchainDesc& desc)
	{
		GfxSwapchain* pSC = new GfxSwapchain();
		pSC->vSync = desc.vSync;
		pSC->imageCount = desc.imageCount;

		// Those mfs are somehow wants to use BOOL and TRUE/FALSE instead of bool and true/false
		BOOL allowTearing = FALSE;
		if (SUCCEEDED(pContext->pFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
			&allowTearing, sizeof(allowTearing))))
		{
			pSC->bAllowTearing = allowTearing == TRUE;
		}

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
		scDesc.Flags = pSC->bAllowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		IDXGISwapChain1* pSwapchain1 = nullptr;
		HRESULT bResult = pContext->pFactory->CreateSwapChainForHwnd(pQueue->pQueue,
			(HWND)desc.pWindowHandle, &scDesc, nullptr, nullptr, &pSwapchain1);
		CHECK_HR(bResult, "IDXGISwapChain1 - CreateSwapChainForHwnd");

		bResult = pSwapchain1->QueryInterface(IID_PPV_ARGS(&pSC->pSwapchain));
		CHECK_HR(bResult, "IDXGISwapChain4 - QueryInterface");
		pSwapchain1->Release();

		pSC->images.resize(desc.imageCount);
		for (u32 i = 0; i < desc.imageCount; i++)
		{
			GfxTexture& image = pSC->images[i];

			bResult = pSC->pSwapchain->GetBuffer(i, IID_PPV_ARGS(&image.pResource));
			CHECK_HR(bResult, "IDXGISwapChain4 - GetBuffer");

			image.pMemory = nullptr;
			image.state = D3D12_RESOURCE_STATE_PRESENT;
			image.format = scDesc.Format;
			image.type = GfxTextureType::Tex2D;
			image.width = desc.width;
			image.height = desc.height;
			image.bIsBackbuffer = true;
		}

		return pSC;
	}

	GfxTexture* Gfx::RequestTexture(GfxSwapchain* pSwapchain, usize index)
	{
		return &pSwapchain->images[index];
	}

	u32 Gfx::GetBackbufferIndex(const GfxSwapchain* pSwapchain)
	{
		return pSwapchain->pSwapchain->GetCurrentBackBufferIndex();
	}

	void Gfx::Present(GfxSwapchain* scHandl)
	{
		const u32 syncInterval = scHandl->vSync ? 1 : 0;
		const u32 flags = (!scHandl->vSync && scHandl->bAllowTearing) ? DXGI_PRESENT_ALLOW_TEARING : 0;

		HRESULT bResult = scHandl->pSwapchain->Present(syncInterval, flags);
		CHECK_REASON(bResult, "IDXGISwapChain4 - Present");
	}

	void Gfx::DestroyGfxSwapchain(GfxSwapchain* scHandl)
	{
		for (GfxTexture& image : scHandl->images)
		{
			if (image.pResource)
				image.pResource->Release();
		}

		if (scHandl->pSwapchain)
			scHandl->pSwapchain->Release();

		delete scHandl;
	}
}