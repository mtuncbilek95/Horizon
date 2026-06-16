#pragma once

#include <Runtime/Graphics/RHI/Texture/GfxTexture.h>
#include <Runtime/Graphics/D3D12/D3D12Backend.h>

namespace Horizon
{
	class D3D12Device;

	class D3D12Texture final: public GfxTexture
	{
		friend class D3D12Device;
	public:
		ID3D12Resource* GetResource() const { return m_resource; }

		D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const { return m_rtvHandle; }
		D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const { return m_dsvHandle; }
		DXGI_FORMAT GetDXGIFormat() const { return m_dxgiFormat; }

	protected:
		~D3D12Texture() final;

	protected:
		ID3D12Resource* m_resource = nullptr;
		D3D12MA::Allocation* m_allocation = nullptr;
		DXGI_FORMAT m_dxgiFormat = DXGI_FORMAT_UNKNOWN;

		u32 m_targetViewIndex = kInvalid32;
		u32 m_depthViewIndex = kInvalid32;
		D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle = {};

		b8 m_isBackbuffer = false;
	};
}