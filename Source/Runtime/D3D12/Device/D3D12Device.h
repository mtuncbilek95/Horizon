#pragma once

#include <Runtime/RHI/Device/Device.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comdef.h>
#include <D3D12MemAlloc.h>

namespace Horizon::RHI
{
	class D3D12Device final : public Device
	{
	public:
		D3D12Device();
		~D3D12Device();

		b8 Initialize(const DeviceDesc& desc);
		void CloseAll();

		UniquePtr<Queue> CreateRHIQueue(const QueueDesc& desc) final;
		UniquePtr<Buffer> CreateRHIBuffer(const BufferDesc& desc) final;
		UniquePtr<Texture> CreateRHITexture(const TextureDesc& desc) final;
		UniquePtr<TextureView> CreateRHITextureView(const TextureViewDesc& desc) final;
		UniquePtr<Sampler> CreateRHISampler(const SamplerDesc& desc) final;
		UniquePtr<ShaderModule> CreateRHIShaderModule(const ShaderModuleDesc& desc) final;
		UniquePtr<PipelineLayout> CreateRHIPipelineLayout(const PipelineLayoutDesc& desc) final;
		UniquePtr<GraphicsPipeline> CreateRHIPipeline(const GraphicsPipelineDesc& desc) final;
		UniquePtr<ComputePipeline> CreateRHIPipeline(const ComputePipelineDesc& desc) final;
		UniquePtr<DescriptorHeap> CreateRHIDescriptorHeap(const DescriptorHeapDesc& desc) final;
		UniquePtr<DescriptorTable> CreateRHIDescriptorTable(const DescriptorTableDesc& desc) final;
		UniquePtr<CommandPool> CreateRHICommandPool(const CommandPoolDesc& desc) final;
		UniquePtr<CommandList> CreateRHICommandList(const CommandListDesc& desc) final;
		UniquePtr<Fence> CreateRHIFence(const FenceDesc& desc) final;
		UniquePtr<QueryPool> CreateRHIQueryPool(const QueryPoolDesc& desc) final;
		UniquePtr<Swapchain> CreateRHISwapchain(const SwapchainDesc& desc) final;

	private:
		void RegisterTerminalMessage();

	private:
		ID3D12Device10* m_device = nullptr;
		IDXGIFactory7* m_factory = nullptr;
		IDXGIAdapter4* m_adapter = nullptr;
		ID3D12Debug1* m_debug = nullptr;

		ID3D12InfoQueue1* m_infoQueue = nullptr;
		DWORD m_callbackCookie = 0;

		D3D12MA::Allocator* m_allocator = nullptr;
	};
}