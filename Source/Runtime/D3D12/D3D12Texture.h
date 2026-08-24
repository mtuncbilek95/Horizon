#pragma once

#include <Runtime/RHI/Texture/GfxTexture.h>
#include <Runtime/D3D12/D3D12Helpers.h>

namespace Horizon::RHI
{
	class D3D12Device;
	class D3D12DescriptorHeap;
	class D3D12Swapchain;

	class D3D12Texture final : public GfxTexture
	{
		friend class D3D12Device;
		friend class D3D12DescriptorHeap;
		friend class D3D12Swapchain;
	public:
		~D3D12Texture() final;

		ID3D12Resource* Handle() const { return m_resource; }
		DXGI_FORMAT Format() const { return m_format; }

		D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetHandle() const { return m_renderTargetHandle; }
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilHandle() const { return m_depthStencilHandle; }

	private:
		ID3D12Resource* m_resource = nullptr;
		D3D12MA::Allocation* m_allocation = nullptr;

		DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;

		D3D12_CPU_DESCRIPTOR_HANDLE m_renderTargetHandle{};
		D3D12_CPU_DESCRIPTOR_HANDLE m_depthStencilHandle{};
	};
}
