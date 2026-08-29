#include "D3D12Swapchain.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Definitions/Allocator.h>
#include <Runtime/Log/Terminal.h>

#include <Runtime/D3D12/D3D12Device.h>
#include <Runtime/D3D12/D3D12DescriptorHeap.h>
#include <Runtime/D3D12/D3D12Texture.h>

#include <Runtime/RHI/Fence/GfxFence.h>
#include <Runtime/RHI/Queue/GfxQueue.h>

namespace Horizon::RHI
{
	D3D12Swapchain::~D3D12Swapchain()
	{
		ReleaseImages();

		if (m_swapchain)
			m_swapchain->Release();
	}

	void D3D12Swapchain::AcquireImages()
	{
		auto* pColorHeap = static_cast<D3D12DescriptorHeap*>(m_desc.pColorHeap);

		m_images.Resize(m_desc.imageCount);

		for (u32 i = 0; i < m_desc.imageCount; i++)
		{
			auto* pImage = Memory::Allocator::Create<D3D12Texture>(Memory::CurrLoc());

			pImage->m_ownerDevice = m_device;
			pImage->m_format = Helpers::ToFormat(m_desc.format);

			pImage->m_desc.type = GfxTextureType::Tex2D;
			pImage->m_desc.format = m_desc.format;
			pImage->m_desc.usage = GfxTextureUsage::RenderTarget;
			pImage->m_desc.width = m_desc.width;
			pImage->m_desc.height = m_desc.height;

			HRESULT hr = m_swapchain->GetBuffer(i, IID_PPV_ARGS(&pImage->m_resource));
			CHECK_HR(hr, "IDXGISwapChain4 - GetBuffer");

			m_images[i] = pImage;
			pColorHeap->CreateRenderTargetView(pImage);
		}
	}

	void D3D12Swapchain::ReleaseImages()
	{
		for (D3D12Texture* pImage : m_images)
		{
			if (!pImage)
				continue;

			Memory::Allocator::Delete(pImage);
		}

		m_images.Clear();
	}

	GfxTexture* D3D12Swapchain::GetImage(u32 index) const
	{
		if (index >= m_images.GetCount())
		{
			Terminal::Error(StringOps::GetName(this), "{} image index is out of range, count {}", index, m_images.GetCount());
			return nullptr;
		}

		return m_images[index];
	}

	b8 D3D12Swapchain::AcquireNextImage(GfxFence* pFence)
	{
		if (m_images.IsEmpty())
		{
			Terminal::Error(StringOps::GetName(this), "Swapchain has no images to acquire");
			return false;
		}

		m_imageIndex = m_swapchain->GetCurrentBackBufferIndex();
		pFence->WaitCPU(m_imageFenceValues[m_imageIndex]);

		return true;
	}

	void D3D12Swapchain::Present(GfxQueue* pQueue, GfxFence* pFence)
	{
		HRESULT hr = m_swapchain->Present(m_syncInterval, m_presentFlags);
		CHECK_REASON(hr, "IDXGISwapChain4 - Present");

		m_imageFenceValues[m_imageIndex] = pQueue->Signal(pFence);
	}

	void D3D12Swapchain::Resize(u32 width, u32 height)
	{
		if (width == 0 || height == 0)
			return;

		m_device->WaitIdle();
		ReleaseImages();

		const DXGI_FORMAT format = Helpers::ToSwapchainFormat(Helpers::ToFormat(m_desc.format));

		HRESULT hr = m_swapchain->ResizeBuffers(m_desc.imageCount, width, height, format,
			DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
		CHECK_HR(hr, "IDXGISwapChain4 - ResizeBuffers");

		m_desc.width = width;
		m_desc.height = height;

		for (u32 i = 0; i < kMaxSwapchainImages; i++)
			m_imageFenceValues[i] = 0;

		AcquireImages();

		m_imageIndex = m_swapchain->GetCurrentBackBufferIndex();
	}
}
