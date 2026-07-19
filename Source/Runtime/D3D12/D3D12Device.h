#pragma once

#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/D3D12/Utils/D3D12Helpers.h>

namespace Horizon
{
	class D3D12DescriptorHeap;
	class D3D12Texture;
	class D3D12Buffer;

	class D3D12Device final : public GfxDevice
	{
	public:
		void Init(const GfxDeviceDesc& desc);
		~D3D12Device() final;

		void InitializeImGui(GfxQueue* pQueue, GfxTextureFormat fmt = GfxTextureFormat::RGBA8) final;
		void ShutdownImGui() final;

		GfxTexture* CreateTexture(const GfxTextureDesc& desc) final;
		GfxBuffer* CreateBuffer(const GfxBufferDesc& desc) final;
		GfxSampler* CreateSampler(const GfxSamplerDesc& desc) final;
		GfxPipeline* CreatePipeline(const GfxGraphicsPipelineDesc& desc) final;
		GfxPipeline* CreatePipeline(const GfxComputePipelineDesc& desc) final;

		GfxQueue* CreateQueue(GfxQueueType type) final;
		GfxFence* CreateFence() final;
		GfxCommandList* CreateCommandList(GfxQueueType type) final;
		GfxSwapchain* CreateSwapchain(const GfxSwapchainDesc& desc, GfxQueue* pPresentQueue) final;

		GfxDescriptorHeap* GetDescriptorHeap(GfxDescriptorHeapType type) final;
		GfxAdapterInfo GetAdapterInfo() const final;
		GfxMemoryStats QueryMemoryStats() final;

		ID3D12Device10* Handle() const { return m_device; }
		ID3D12RootSignature* GetRootSignature() const { return m_rootSignature; }
		ID3D12CommandSignature* GetDrawSignature() const { return m_drawSignature; }
		ID3D12CommandSignature* GetDrawIndexedSignature() const { return m_drawIndexedSignature; }
		ID3D12CommandSignature* GetDispatchSignature() const { return m_dispatchSignature; }

		D3D12Texture* CreateBackbufferTexture(ID3D12Resource* pResource, u32 width, u32 height, DXGI_FORMAT format);
		void DestroyBackbufferTexture(D3D12Texture* pTexture);

	private:
		D3D12DescriptorHeap* CreateDescriptorHeap(GfxDescriptorHeapType type, u32 capacity, b8 shaderVisible);

		void CreateTextureSRV(D3D12Texture* pTex);
		void CreateTextureUAV(D3D12Texture* pTex);
		void CreateTextureRTV(D3D12Texture* pTex);
		void CreateTextureDSV(D3D12Texture* pTex);

		void CreateBufferSRV(D3D12Buffer* pBuf);
		void CreateBufferUAV(D3D12Buffer* pBuf);

		void CreateRootSignature();
		void CreateCommandSignatures();
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

		D3D12DescriptorHeap* m_resourceHeap = nullptr;
		D3D12DescriptorHeap* m_samplerHeap = nullptr;
		D3D12DescriptorHeap* m_colorHeap = nullptr;
		D3D12DescriptorHeap* m_depthHeap = nullptr;

		ID3D12RootSignature* m_rootSignature = nullptr;
		ID3D12CommandSignature* m_drawSignature = nullptr;
		ID3D12CommandSignature* m_drawIndexedSignature = nullptr;
		ID3D12CommandSignature* m_dispatchSignature = nullptr;
	};
}