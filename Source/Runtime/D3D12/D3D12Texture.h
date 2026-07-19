#pragma once

#include <Runtime/RHI/Texture/GfxTexture.h>
#include <Runtime/D3D12/Utils/D3D12Helpers.h>

namespace Horizon
{
	class D3D12Device;

	class D3D12Texture final : public GfxTexture
	{
		friend class D3D12Device;
	public:
		~D3D12Texture() final;

		void SetDebugName(const char* pName) final;

		ID3D12Resource* GetResource() const { return m_resource; }
		D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const { return m_rtvHandle; }
		D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const { return m_dsvHandle; }
		DXGI_FORMAT GetDXGIFormat() const { return m_dxgiFormat; }

	private:
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