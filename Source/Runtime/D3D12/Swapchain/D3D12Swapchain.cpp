#include "D3D12Swapchain.h"

#include <Runtime/D3D12/Helpers/D3D12Helpers.h>
#include <Runtime/D3D12/Texture/D3D12Texture.h>
#include <Runtime/RHI/Device/Device.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

#include <cstdio>
#include <utility>

namespace Horizon::RHI
{
	D3D12Swapchain::~D3D12Swapchain()
	{
		ReleaseImages();

		if (m_waitableObject)
			CloseHandle(m_waitableObject);

		if (m_swapchain)
			m_swapchain->Release();
	}

	b8 D3D12Swapchain::AcquireNextImage()
	{
		if (m_waitableObject && WaitForSingleObjectEx(m_waitableObject, 1000, TRUE) == WAIT_TIMEOUT)
			Terminal::Warn(StringHelper::GetTypeName(this), "Frame latency waitable object timed out");

		m_imageIndex = m_swapchain->GetCurrentBackBufferIndex();
		return true;
	}

	void D3D12Swapchain::Present()
	{
		b8 isImmediate = m_presentMode == PresentMode::Immediate;
		UINT syncInterval = isImmediate ? 0 : 1;
		UINT presentFlags = isImmediate && m_tearingSupported ? DXGI_PRESENT_ALLOW_TEARING : 0;

		HRESULT hr = m_swapchain->Present(syncInterval, presentFlags);
		if (FAILED(hr))
			Terminal::Error(StringHelper::GetTypeName(this), "Present failed: 0x{:08X}", static_cast<u32>(hr));
	}

	void D3D12Swapchain::Resize(u32 width, u32 height)
	{
		if (width == 0 || height == 0 || (width == m_width && height == m_height))
			return;

		ReleaseImages();

		HRESULT hr = m_swapchain->ResizeBuffers(m_imageCount, width, height, Helpers::ToFormat(m_format), m_swapchainFlags);
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "ResizeBuffers failed: 0x{:08X}", static_cast<u32>(hr));
			return;
		}

		m_width = width;
		m_height = height;

		CreateImages();
		m_imageIndex = m_swapchain->GetCurrentBackBufferIndex();
	}

	void D3D12Swapchain::SetDebugName(const c8* pName)
	{
		if (!pName)
			return;

		c8 buffer[128];
		for (u32 i = 0; i < m_imageCount; i++)
		{
			std::snprintf(buffer, sizeof(buffer), "%s[%u]", pName, i);
			m_images[i]->SetDebugName(buffer);
		}
	}

	b8 D3D12Swapchain::CreateImages()
	{
		for (u32 i = 0; i < m_imageCount; i++)
		{
			UniquePtr<D3D12Texture> pImage = MakeUnique<D3D12Texture>();

			pImage->m_ownerDevice = m_ownerDevice;
			pImage->m_type = TextureType::Texture2D;
			pImage->m_format = m_format;
			pImage->m_usage = TextureUsage::RenderTarget;
			pImage->m_width = m_width;
			pImage->m_height = m_height;

			HRESULT hr = m_swapchain->GetBuffer(i, IID_PPV_ARGS(&pImage->m_resource));
			if (FAILED(hr))
			{
				Terminal::Error(StringHelper::GetTypeName(this), "GetBuffer {} failed: 0x{:08X}", i, static_cast<u32>(hr));
				return false;
			}

			TextureViewDesc viewDesc = {};
			viewDesc.pTexture = pImage.Get();
			viewDesc.type = TextureViewType::Texture2D;

			UniquePtr<TextureView> pView = m_ownerDevice->CreateRHITextureView(viewDesc);
			if (!pView)
				return false;

			m_images[i] = std::move(pImage);
			m_views[i] = std::move(pView);
		}

		return true;
	}

	void D3D12Swapchain::ReleaseImages()
	{
		for (u32 i = 0; i < kMaxSwapchainImages; i++)
		{
			m_views[i] = nullptr;
			m_images[i] = nullptr;
		}
	}
}
