#pragma once

#include <Runtime/RHI/Device/Device.h>
#include <Runtime/D3D12/Descriptor/D3D12CpuDescriptorPool.h>

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
		UniquePtr<Swapchain> CreateRHISwapchain(const SwapchainDesc& desc) final;

		void Submit(QueueType type, ReadOnlyList<CommandList*> lists) final;
		u64 Signal(QueueType type, Fence* pFence) final;
		void Wait(QueueType type, Fence* pFence, u64 value) final;
		void WaitIdle() final;

		u64 GetTimestampFrequency(QueueType type) final;

		void InitializeImGui(u32 framesInFlight, DescriptorHeap* pResourceHeap, TextureFormat colorFormat) final;
		void NewFrameImGui() final;
		void ShutdownImGui() final;

		ID3D12Device10* Handle() const { return m_device; }
		ID3D12CommandQueue* QueueHandle(QueueType type) const { return m_queues[static_cast<usize>(type)]; }

		ID3D12CommandSignature* DrawSignature() const { return m_drawSignature; }
		ID3D12CommandSignature* DrawIndexedSignature() const { return m_drawIndexedSignature; }
		ID3D12CommandSignature* DispatchSignature() const { return m_dispatchSignature; }
		ID3D12CommandSignature* DispatchMeshSignature() const { return m_dispatchMeshSignature; }

	private:
		void RegisterTerminalMessage();

	private:
		ID3D12Device10* m_device = nullptr;
		ID3D12CommandQueue* m_queues[static_cast<usize>(QueueType::Count)] = {};
		ID3D12Fence* m_idleFence = nullptr;
		u64 m_idleValue = 0;
		IDXGIFactory7* m_factory = nullptr;
		IDXGIAdapter4* m_adapter = nullptr;
		ID3D12Debug1* m_debug = nullptr;

		ID3D12InfoQueue1* m_infoQueue = nullptr;
		DWORD m_callbackCookie = 0;

		D3D12MA::Allocator* m_allocator = nullptr;

		D3D12CpuDescriptorPool m_rtvPool;
		D3D12CpuDescriptorPool m_dsvPool;

		ID3D12CommandSignature* m_drawSignature = nullptr;
		ID3D12CommandSignature* m_drawIndexedSignature = nullptr;
		ID3D12CommandSignature* m_dispatchSignature = nullptr;
		ID3D12CommandSignature* m_dispatchMeshSignature = nullptr;
	};
}