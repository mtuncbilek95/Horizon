#pragma once

#include <Runtime/RHI/Swapchain/GfxSwapchain.h>
#include <Runtime/D3D12/Utils/D3D12Helpers.h>

#include <vector>

namespace Horizon
{
	class D3D12Device;
	class D3D12Texture;

	class D3D12Swapchain final : public GfxSwapchain
	{
		friend class D3D12Device;
	public:
		~D3D12Swapchain() final;

		GfxTexture* GetBackbuffer(u32 index) final;
		u32 GetCurrentIndex() final;
		void Present() final;
		void Resize(u32 width, u32 height) final;

	private:
		void AcquireBackbuffers();
		void ReleaseBackbuffers();

	private:
		IDXGISwapChain4* m_swapchain = nullptr;
		D3D12Device* m_device = nullptr;

		std::vector<D3D12Texture*> m_backbuffers;

		b8 m_allowTearing = false;
	};
}