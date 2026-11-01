#pragma once

#include <Runtime/RHI/Texture/TextureView.h>

#include <dxgi1_6.h>
#include <d3d12.h>

namespace Horizon::RHI
{
	class D3D12CpuDescriptorPool;

	class D3D12TextureView final : public TextureView
	{
		friend class D3D12Device;
	public:
		~D3D12TextureView() final;

		D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetHandle() const { return m_rtvHandle; }
		D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilHandle() const { return m_dsvHandle; }

		const D3D12_SHADER_RESOURCE_VIEW_DESC& ShaderResourceDesc() const { return m_srvDesc; }
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& UnorderedAccessDesc() const { return m_uavDesc; }

	private:
		D3D12CpuDescriptorPool* m_rtvPool = nullptr;
		D3D12CpuDescriptorPool* m_dsvPool = nullptr;

		D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle = {};

		D3D12_SHADER_RESOURCE_VIEW_DESC m_srvDesc = {};
		D3D12_UNORDERED_ACCESS_VIEW_DESC m_uavDesc = {};
	};
}