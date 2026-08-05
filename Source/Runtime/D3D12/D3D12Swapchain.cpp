#include "D3D12Swapchain.h"

#include <Runtime/D3D12/D3D12Device.h>
#include <Runtime/D3D12/D3D12Texture.h>

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
		const u32 syncInterval = m_desc.vSync ? 1 : 0;
		const u32 presentFlags = (!m_desc.vSync && m_allowTearing) ? DXGI_PRESENT_ALLOW_TEARING : 0;

		HRESULT hr = m_swapchain->Present(syncInterval, presentFlags);
		CHECK_REASON(hr, "IDXGISwapChain4 - Present");
	}

	void D3D12Swapchain::Resize(u32 width, u32 height)
	{
		if (width == m_desc.width && height == m_desc.height)
			return;

		ReleaseBackbuffers();

		const u32 resizeFlags = m_allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		HRESULT hr = m_swapchain->ResizeBuffers(m_desc.imageCount, width, height,
			Helpers::ToDXGIFormat(m_desc.format), resizeFlags);
		CHECK_HR(hr, "IDXGISwapChain4 - ResizeBuffers");

		m_desc.width = width;
		m_desc.height = height;

		AcquireBackbuffers();
	}

	void D3D12Swapchain::AcquireBackbuffers()
	{
		m_backbuffers.Resize(m_desc.imageCount);

		for (u32 index = 0; index < m_desc.imageCount; index++)
		{
			ID3D12Resource* resource = nullptr;

			HRESULT hr = m_swapchain->GetBuffer(index, IID_PPV_ARGS(&resource));
			CHECK_HR(hr, "IDXGISwapChain4 - GetBuffer");

			m_backbuffers[index] = m_device->CreateBackbufferTexture(resource, m_desc.width, m_desc.height,
				Helpers::ToDXGIFormat(m_desc.format));
		}
	}

	void D3D12Swapchain::ReleaseBackbuffers()
	{
		for (D3D12Texture* pBackbuffer : m_backbuffers)
			m_device->DestroyBackbufferTexture(pBackbuffer);

		m_backbuffers.Clear();
	}
}