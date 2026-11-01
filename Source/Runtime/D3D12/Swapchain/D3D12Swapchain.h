#pragma once

#include <Runtime/RHI/Swapchain/Swapchain.h>

#include <dxgi1_6.h>
#include <d3d12.h>

namespace Horizon::RHI
{
	class D3D12Swapchain final : public Swapchain
	{
		friend class D3D12Device;
	public:
		~D3D12Swapchain() final;

		b8 AcquireNextImage() final;
		void Present() final;
		void Resize(u32 width, u32 height) final;

		void SetDebugName(const c8* pName) final;

		IDXGISwapChain4* Handle() const { return m_swapchain; }

	private:
		b8 CreateImages();
		void ReleaseImages();

	private:
		IDXGISwapChain4* m_swapchain = nullptr;
		HANDLE m_waitableObject = nullptr;
		UINT m_swapchainFlags = 0;
		b8 m_tearingSupported = false;
	};
}
