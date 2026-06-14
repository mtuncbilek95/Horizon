#include "D3D12Swapchain.h"

#include "D3D12Device.h"
#include "D3D12Texture.h"

namespace Horizon
{
	D3D12Swapchain::~D3D12Swapchain()
	{
		ReleaseBackbuffers();

		if (m_swapchain)
			m_swapchain->Release();
	}

	GfxTexture* D3D12Swapchain::GetBackbuffer(u32 index)
	{
		return m_backbuffers[index];
	}

	u32 D3D12Swapchain::GetCurrentIndex()
	{
		return m_swapchain->GetCurrentBackBufferIndex();
	}

	void D3D12Swapchain::Present()
	{
		const u32 syncInterval = m_vSync ? 1 : 0;
		const u32 presentFlags = (!m_vSync && m_allowTearing) ? DXGI_PRESENT_ALLOW_TEARING : 0;

		HRESULT hr = m_swapchain->Present(syncInterval, presentFlags);
		CHECK_REASON(hr, "IDXGISwapChain4 - Present");
	}

	void D3D12Swapchain::Resize(u32 width, u32 height)
	{
		if (width == m_width && height == m_height)
			return;

		ReleaseBackbuffers();

		const u32 resizeFlags = m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		HRESULT hr = m_swapchain->ResizeBuffers(m_imageCount, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, resizeFlags);
		CHECK_HR(hr, "IDXGISwapChain4 - ResizeBuffers");

		m_width = width;
		m_height = height;

		AcquireBackbuffers();
	}

	void D3D12Swapchain::AcquireBackbuffers()
	{
		m_backbuffers.resize(m_imageCount);

		for (u32 index = 0; index < m_imageCount; index++)
		{
			ID3D12Resource* resource = nullptr;
			HRESULT hr = m_swapchain->GetBuffer(index, IID_PPV_ARGS(&resource));
			CHECK_HR(hr, "IDXGISwapChain4 - GetBuffer");

			m_backbuffers[index] = m_device->CreateBackbufferTexture(resource, m_width, m_height,
				DXGI_FORMAT_R8G8B8A8_UNORM);
		}
	}

	void D3D12Swapchain::ReleaseBackbuffers()
	{
		for (D3D12Texture* backbuffer : m_backbuffers)
			m_device->DestroyBackbufferTexture(backbuffer);

		m_backbuffers.clear();
	}
}
