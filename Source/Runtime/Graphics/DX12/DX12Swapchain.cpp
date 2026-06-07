#include <Runtime/Graphics/RHI/GfxSwapchain.h>
#include <Runtime/Graphics/RHI/GfxDevice.h>
#include <Runtime/Graphics/RHI/GfxTexture.h>

#include <Runtime/Graphics/DX12/DX12Device.h>
#include <Runtime/Graphics/DX12/DX12Utils.h>

#define CHECK_HR(hr, what) \
	if(FAILED(hr)) { Horizon::MainLog::Error("{}: {}", what, _com_error(hr).ErrorMessage()); exit(-1); }

namespace Horizon
{
	GfxSwapchain::GfxSwapchain(const GfxSwapchainDesc& desc, GfxDevice* pDevice)
	{
		m_device = pDevice;
		m_imageCount = desc.imageCount;
		m_width = desc.width;
		m_height = desc.height;
		m_format = desc.format;
		m_vsync = desc.vsync;

		DX12Device* deviceNative = static_cast<DX12Device*>(pDevice->GetNative());

		BOOL allowTearing = FALSE;
		deviceNative->factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
		m_allowTearing = allowTearing;

		DXGI_SWAP_CHAIN_DESC1 scDesc = {};
		scDesc.Width = desc.width;
		scDesc.Height = desc.height;
		scDesc.Format = ToDXGI(desc.format);
		scDesc.SampleDesc = { 1, 0 };
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.BufferCount = desc.imageCount;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scDesc.Scaling = DXGI_SCALING_STRETCH;
		scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		scDesc.Flags = m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		IDXGISwapChain1* sc1 = nullptr;
		CHECK_HR(deviceNative->factory->CreateSwapChainForHwnd(deviceNative->queues[u32(GfxQueueType::Graphics)],
			static_cast<HWND>(desc.windowHandle), &scDesc, nullptr, nullptr, &sc1), "CreateSwapChainForHwnd");

		IDXGISwapChain4* sc4 = nullptr;
		CHECK_HR(sc1->QueryInterface(IID_PPV_ARGS(&sc4)), "SwapChain1 -> SwapChain4");
		sc1->Release();

		m_native = sc4;

		CreateBackbuffers();
	}

	GfxSwapchain::~GfxSwapchain()
	{
		m_device->WaitIdle();
		m_backbuffers.clear();   // backbuffer dtor'ları resource + rtv slot serbest bırakır

		if (m_native)
			static_cast<IDXGISwapChain4*>(m_native)->Release();
	}

	void GfxSwapchain::CreateBackbuffers()
	{
		IDXGISwapChain4* sc = static_cast<IDXGISwapChain4*>(m_native);

		GfxTextureDesc bbDesc = {};
		bbDesc.width = m_width;
		bbDesc.height = m_height;
		bbDesc.format = m_format;
		bbDesc.usage = GfxTextureUsage::RenderTarget;
		bbDesc.type = GfxTextureType::Tex2D;

		m_backbuffers.resize(m_imageCount);
		for (u32 i = 0; i < m_imageCount; ++i)
		{
			ID3D12Resource* res = nullptr;
			CHECK_HR(sc->GetBuffer(i, IID_PPV_ARGS(&res)), "Swapchain GetBuffer");
			m_backbuffers[i] = std::make_unique<GfxTexture>(m_device, static_cast<GfxHandle>(res), bbDesc);
		}
	}

	GfxTexture* GfxSwapchain::AcquireNext()
	{
		IDXGISwapChain4* sc = static_cast<IDXGISwapChain4*>(m_native);
		m_backbufferIndex = sc->GetCurrentBackBufferIndex();
		return m_backbuffers[m_backbufferIndex].get();
	}

	void GfxSwapchain::Present()
	{
		IDXGISwapChain4* sc = static_cast<IDXGISwapChain4*>(m_native);

		if (!m_vsync && m_allowTearing)
			sc->Present(0, DXGI_PRESENT_ALLOW_TEARING);
		else
			sc->Present(m_vsync ? 1 : 0, 0);
	}

	void GfxSwapchain::Resize(u32 width, u32 height)
	{
		if (width == 0 || height == 0)
			return;

		m_device->WaitIdle();
		m_backbuffers.clear();   // immediate cleanup (ResizeBuffers öncesi tüm ref'ler bırakılmalı)

		m_width = width;
		m_height = height;

		IDXGISwapChain4* sc = static_cast<IDXGISwapChain4*>(m_native);
		UINT flags = m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		CHECK_HR(sc->ResizeBuffers(m_imageCount, width, height, ToDXGI(m_format), flags), "ResizeBuffers");

		CreateBackbuffers();
	}
}
