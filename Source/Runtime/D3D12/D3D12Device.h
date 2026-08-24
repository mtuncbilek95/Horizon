#pragma once

#include <Runtime/RHI/Device/GfxDevice.h>
#include <Runtime/D3D12/D3D12Helpers.h>

#include <Runtime/Containers/List.h>

namespace Horizon::RHI
{
	class D3D12Queue;

	class H_EXPORT D3D12Device final : public GfxDevice
	{
	public:
		~D3D12Device() final;

		void Init(const GfxDeviceDesc& desc);

		void InitializeImGui(u32 maxFrames, GfxQueue* pQueue, GfxDescriptorHeap* pHeap, GfxTextureFormat colorFormat) final;
		void NewFrameImGui() final;
		void ShutdownImGui() final;

		GfxDescriptorHeap* CreateDescriptorHeap(const GfxDescriptorHeapDesc& desc) final;
		GfxSwapchain* CreateSwapchain(const GfxSwapchainDesc& desc, GfxQueue* pPresentQueue) final;

		GfxTexture* CreateTexture(const GfxTextureDesc& desc) final;
		GfxBuffer* CreateBuffer(const GfxBufferDesc& desc) final;
		GfxBufferArena* CreateBufferArena(const GfxBufferArenaDesc& desc) final;
		GfxUploadRing* CreateUploadRing(const GfxUploadRingDesc& desc) final;
		GfxShader* CreateShader(const GfxShaderDesc& desc) final;
		GfxPipeline* CreatePipeline(const GfxGraphicsPipelineDesc& desc) final;
		GfxPipeline* CreatePipeline(const GfxComputePipelineDesc& desc) final;

		GfxCommandList* CreateCommandList(GfxQueueType type) final;
		GfxQueue* CreateQueue(GfxQueueType type) final;
		GfxFence* CreateFence() final;

		void WaitIdle() final;

		ID3D12Device10* Handle() const { return m_device; }
		IDXGIFactory7* Factory() const { return m_factory; }
		D3D12MA::Allocator* MemoryAllocator() const { return m_allocator; }
		ID3D12RootSignature* RootSignature() const { return m_rootSignature; }

		ID3D12CommandSignature* DrawSignature() const { return m_drawSignature; }
		ID3D12CommandSignature* DrawIndexedSignature() const { return m_drawIndexedSignature; }
		ID3D12CommandSignature* DispatchSignature() const { return m_dispatchSignature; }

		void ForgetQueue(D3D12Queue* pQueue);

	private:
		void CreateRootSignature();
		void CreateCommandSignatures();
		void CreateTerminalLog();

	private:
		ID3D12Device10* m_device = nullptr;
		IDXGIFactory7* m_factory = nullptr;
		IDXGIAdapter4* m_adapter = nullptr;
		ID3D12Debug1* m_debug = nullptr;

		ID3D12InfoQueue1* m_infoQueue = nullptr;
		DWORD m_callbackCookie = 0;

		D3D12MA::Allocator* m_allocator = nullptr;

		ID3D12RootSignature* m_rootSignature = nullptr;

		ID3D12CommandSignature* m_drawSignature = nullptr;
		ID3D12CommandSignature* m_drawIndexedSignature = nullptr;
		ID3D12CommandSignature* m_dispatchSignature = nullptr;

		ID3D12Fence* m_idleFence = nullptr;
		u64 m_idleValue = 0;

		List<D3D12Queue*> m_queues;
	};
}
