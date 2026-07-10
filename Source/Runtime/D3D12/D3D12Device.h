#pragma once

#include <Runtime/RHI/Device/GfxDevice.h>

#include <Runtime/D3D12/D3D12Backend.h>
#include <Runtime/D3D12/D3D12DescriptorHeap.h>

namespace Horizon
{
	class D3D12Texture;
	class D3D12Buffer;

	class D3D12Device final : public GfxDevice
	{
	public:
		void Init();
		~D3D12Device() final;

		void InitializeImGui(GfxQueue* pQueue, GfxTextureFormat fmt = GfxTextureFormat::RGBA8, u32 framesInFlight = MaxFramesInFlight) final;
		void ShutdownImGui() final;

		GfxTexture* CreateTexture(const GfxTextureDesc& desc) final;
		GfxBuffer* CreateBuffer(const GfxBufferDesc& desc) final;
		GfxPipeline* CreatePipeline(const GfxGraphicsPipelineDesc& desc) final;
		GfxPipeline* CreatePipeline(const GfxComputePipelineDesc& desc) final;

		GfxQueue* CreateQueue(GfxQueueType type) final;
		GfxFence* CreateFence() final;
		GfxCommandList* CreateCommandList(GfxQueueType type) final;
		GfxSwapchain* CreateSwapchain(const GfxSwapchainDesc& desc, GfxQueue* presentQueue) final;

		void FreeDescriptor(GfxDescriptorHeapType type, u32 index) final { HeapFor(type).Free(index); }

		ID3D12RootSignature* GetRootSignature() const { return m_rootSignature; }
		ID3D12DescriptorHeap* GetResourceHeap() const { return m_resourceHeap.pHeap; }

		D3D12DescriptorHeap* GetResourceHeapRHI() { return &m_resourceHeap; }

		D3D12Texture* CreateBackbufferTexture(ID3D12Resource* resource, u32 width, u32 height, DXGI_FORMAT format);
		void DestroyBackbufferTexture(D3D12Texture* texture);

	private:
		D3D12DescriptorHeap& HeapFor(GfxDescriptorHeapType type)
		{
			return type == GfxDescriptorHeapType::Color ? m_colorHeap :
				type == GfxDescriptorHeapType::Depth ? m_depthHeap : m_resourceHeap;
		}

		void CreateTextureSRV(D3D12Texture* tex);
		void CreateTextureUAV(D3D12Texture* tex);
		void CreateTextureRTV(D3D12Texture* tex);
		void CreateTextureDSV(D3D12Texture* tex);

		void CreateBufferSRV(D3D12Buffer* buf);
		void CreateBufferUAV(D3D12Buffer* buf);

		void CreateDescriptorHeap(D3D12DescriptorHeap& out, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 capacity, b8 shaderVisible);
		void CreateRootSignature();
#if defined(HORIZON_DEBUG)
		void CreateTerminalLog();
#endif

	private:
		ID3D12Device10* m_device = nullptr;
		IDXGIFactory7* m_factory = nullptr;
		IDXGIAdapter4* m_adapter = nullptr;
		ID3D12Debug* m_debug = nullptr;

#if defined(HORIZON_DEBUG)
		ID3D12InfoQueue1* m_infoQueue = nullptr;
		DWORD m_callbackCookie = 0;
#endif

		D3D12MA::Allocator* m_allocator = nullptr;

		D3D12DescriptorHeap m_resourceHeap;
		D3D12DescriptorHeap m_colorHeap;
		D3D12DescriptorHeap m_depthHeap;

		ID3D12RootSignature* m_rootSignature = nullptr;
	};
}