#pragma once

#include <Runtime/Core/SlotMap.h>
#include <Runtime/Log/Log.h>
#include <Runtime/Graphics/RHI/GfxContext.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comdef.h>
#include <D3D12MemAlloc.h>

#include <array>
#include <vector>
#include <string_view>
#include <mutex>

namespace Horizon
{
#define CHECK_HR(hr, what) \
	if(FAILED(hr)) { MainLog::Error("{}: {}", what, _com_error(hr).ErrorMessage()); exit(-1); }

#define CHECK_REASON(hr, what) \
	if(FAILED(hr)) { MainLog::Error("{}: {}", what, _com_error(hr).ErrorMessage()); }

	struct DX12DescriptorHeap
	{
		ID3D12DescriptorHeap* pHeap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE cpuStart{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpuStart{};
		u32 descriptorSize = 0;
		u32 capacity = 0;

		std::vector<u32> freeList;
		u32 top = 0;
		std::mutex mutex;
	};

	struct DX12Buffer
	{
		ID3D12Resource* pResource = nullptr;
		D3D12MA::Allocation* pMemory = nullptr;
		void* pMapped = nullptr;
		
		GfxMemoryType memory = GfxMemoryType::GPU;

		u64 gpuVirtualAddress = 0;
		usize sizeInBytes = 0;
		u32 shaderViewIndex = ~0u;
		u32 accessViewIndex = ~0u;
	};

	struct DX12UploadEntry
	{
		ID3D12Resource* pSrc; 
		ID3D12Resource* pDst; 
		u64 srcOffset; 
		u64 dstOffset; 
		u64 size;
	};

	struct DX12Texture
	{
		ID3D12Resource* pResource = nullptr;
		D3D12MA::Allocation* pMemory = nullptr;
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		u32 width = 0, height = 0;
		u32 targetViewIndex = ~0u;
		u32 depthViewIndex = ~0u;
		u32 shaderViewIndex = ~0u;
		u32 accessViewIndex = ~0u;
		b8 bIsBackbuffer = false;
	};

	struct DX12Pipeline
	{
		ID3D12PipelineState* pPipeline = nullptr;
		D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		b8 bIsCompute = false;
	};

	struct DX12Fence
	{
		ID3D12Fence* pFence = nullptr;
		HANDLE pEvent = nullptr;
		u64 fenceValue = 0;
	};

	struct DX12CmdList
	{
		ID3D12GraphicsCommandList* pList = nullptr;
		ID3D12CommandAllocator* pAllocator = nullptr;
		u32 frameSlot = 0;
		b8 bRecording = false;
	};

	struct Context
	{
		static constexpr u32 CmdLanes = MaxWorkers * u32(GfxQueueType::Count);

		// ========== DEVICE-DESC ==========
		u32 pCmdWorkerCount = 1;
		u32 imageCount = 2;
		b8 bEnableDebug = false;
		b8 bGPUValidation = false;
		b8 bSyncPresent = false;

		// ========== DEBUGGER ==========
#if defined(HORIZON_DEBUG)
		ID3D12Debug* pDebug = nullptr;
		ID3D12InfoQueue1* pInfoQueue = nullptr;
		DWORD pInfoId = 0;
#endif
		// ========== MAIN-SYST ==========
		IDXGIFactory7* pFactory = nullptr;
		IDXGIAdapter4* pAdapter = nullptr;
		ID3D12Device10* pDevice = nullptr;
		IDXGISwapChain4* pSwapchain = nullptr;
		D3D12MA::Allocator* pAllocator = nullptr;
		ID3D12RootSignature* pGlobalRoot = nullptr;

		// ========== OBJ-POOLS ==========
		ObjectSlotMap<DX12Buffer, GfxBufferHandle, 4096> bufferPool;
		ObjectSlotMap<DX12Texture, GfxTextureHandle, 2048> texturePool;
		ObjectSlotMap<DX12Pipeline, GfxPipelineHandle, 512> pipelinePool;
		ObjectSlotMap<DX12Fence, GfxFenceHandle, 64> fencePool;
		
		std::array<ID3D12CommandQueue*, usize(GfxQueueType::Count)> queuePool;

		std::array<DX12CmdList, CmdLanes* MaxListsPerWorker> cmdLists;
		std::array<ID3D12CommandAllocator*, CmdLanes* MaxFramesInFlight> cmdAllocators{};
		std::array<u32, CmdLanes> nextLocal{};
		u32 currentFrameSlot = 0;

		DX12DescriptorHeap resourceHeap;
		DX12DescriptorHeap renderTargetHeap;
		DX12DescriptorHeap depthStencilHeap;

		std::vector<GfxTextureHandle> backBufferPool;
	};

	Context& DX12Context();

	namespace Helpers
	{
		u32 AllocateDescriptor(DX12DescriptorHeap& heap);
		void FreeDescriptor(DX12DescriptorHeap& heap, u32 index);
		D3D12_CPU_DESCRIPTOR_HANDLE CpuAt(const DX12DescriptorHeap& heap, u32 i);

		void CreateGlobalRootSignature();
		void CreateBackbuffers(u32 width, u32 height);
		void CreateTerminalLog();
		void CreateDescriptorHeap(DX12DescriptorHeap& heap, D3D12_DESCRIPTOR_HEAP_TYPE type,
			u32 capacity, b8 shaderVisible);

		u32 CreateBufferSRV(DX12Buffer& b, const GfxBufferDesc& desc);
		u32 CreateBufferUAV(DX12Buffer& b, const GfxBufferDesc& desc);

		DXGI_FORMAT ToDXGIFormat(GfxTextureFormat format);
		D3D12_RESOURCE_STATES ToResourceState(GfxResourceState state);

		inline DX12CmdList& ResolveCmdList(GfxCmdListHandle handle)
		{
			return DX12Context().cmdLists[handle.Index()];
		}
	}
}