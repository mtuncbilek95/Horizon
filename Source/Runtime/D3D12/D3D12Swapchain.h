#pragma once

#include <Runtime/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/D3D12/D3D12Helpers.h>

#include <Runtime/Containers/List.h>

namespace Horizon::RHI
{
	class D3D12Device;
	class D3D12Texture;

	class D3D12Swapchain final : public GfxSwapchain
	{
		friend class D3D12Device;
	public:
		~D3D12Swapchain() final;

		b8 AcquireNextImage(GfxFence* pFence) final;
		void Present(GfxQueue* pQueue, GfxFence* pFence) final;
		void Resize(u32 width, u32 height) final;

		GfxTexture* GetImage(u32 index) const final;

		IDXGISwapChain4* Handle() const { return m_swapchain; }

	private:
		void AcquireImages();
		void ReleaseImages();

		IDXGISwapChain4* m_swapchain = nullptr;
		D3D12Device* m_device = nullptr;

		List<D3D12Texture*> m_images;

		u32 m_syncInterval = 0;
		u32 m_presentFlags = 0;
	};
}
