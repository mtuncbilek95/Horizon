#pragma once

#include <Engine/Graphics/RHI/GfxDevice.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comdef.h>
#include <D3D12MemAlloc.h>

#include <array>
#include <vector>
#include <string_view>

namespace Horizon
{
	static constexpr u32 MaxFramesInFlight = 2;
	static constexpr u32 MaxWorkers = 8;
	static constexpr u32 BindlessCapacity = 1 << 16;
	static constexpr u32 MaxListsPerWorker = 64;

	struct GfxCommandList
	{
		ID3D12GraphicsCommandList6* list = nullptr;
		ID3D12CommandAllocator* allocator = nullptr;
		GfxQueueType type = GfxQueueType::Graphics;
		b8 computeMode = false;
	};

	struct WorkerCmd
	{
		ID3D12CommandAllocator* allocator = nullptr;
		std::array<GfxCommandList, MaxListsPerWorker> lists;
		u32 listCount = 0;
		u32 nextList = 0;
	};

	struct Frame
	{
		u64 fenceValue = 0;
		std::array<WorkerCmd, MaxWorkers> workers;
	};
}

namespace Horizon::DX12
{
	struct DX12Texture
	{
		ID3D12Resource* resource = nullptr;
		D3D12MA::Allocation* memory = nullptr;
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		u32 width = 0, height = 0;
		u32 targetViewIndex = ~0u;
		u32 depthViewIndex = ~0u;
		u32 shaderViewIndex = ~0u;
		u32 accessViewIndex = ~0u;
		b8 isBackbuffer = false;
	};

	struct DX12Buffer
	{
		ID3D12Resource* resource = nullptr;
		D3D12MA::Allocation* memory = nullptr;
		u64 size = 0;
		D3D12_GPU_VIRTUAL_ADDRESS gpuVA = 0;
		void* mapped = nullptr;
		u32 shaderViewIndex = ~0u;
		u32 accessViewIndex = ~0u;
	};

	struct DX12Pipeline
	{
		ID3D12PipelineState* pso = nullptr;
		b8 isCompute = false;
		D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	};

	struct Context
	{
		IDXGIFactory7* factory = nullptr;
		IDXGIAdapter4* adapter = nullptr;
		ID3D12Device10* device = nullptr;
		IDXGISwapChain4* swapchain = nullptr;

		DXGI_FORMAT swapchainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		std::array<GfxTextureHandle, MaxFramesInFlight> backbuffers;
		u32 backbufferCount = 0, backbufferIndex = 0, nextTargetView = 0, nextDepthView = 0;
		b8 vsync = true;

		std::array<ID3D12CommandQueue*, 3> queues;
		std::vector<DX12Texture> textures;
		std::vector<u32> texGenerations, texFreeList;

		std::vector<DX12Buffer> buffers;
		std::vector<u32> bufGenerations, bufFreeList;
		u32 nextBindless = 0;
		std::vector<u32> bindlessFreeList;

		ID3D12DescriptorHeap* samplerHeap = nullptr;
		u32 samplerStride = 0;
		u32 nextSampler = 0;
		std::vector<u32> samplerFreeList;

		ID3D12RootSignature* rootSignature = nullptr;
		std::vector<DX12Pipeline> pipelines;
		std::vector<u32> pipeGenerations, pipeFreeList;

		std::vector<ID3D12CommandSignature*> cmdSigs;
		std::vector<u32> cmdSigStrides;
		std::vector<u32> cmdSigGenerations, cmdSigFreeList;

		D3D12MA::Allocator* allocator = nullptr;

		ID3D12DescriptorHeap* bindlessHeap = nullptr;
		ID3D12DescriptorHeap* targetViewHeap = nullptr;
		ID3D12DescriptorHeap* depthViewHeap = nullptr;
		u32 bindlessStride = 0, targetViewStride = 0, depthViewStride = 0;

		ID3D12Fence* frameFence = nullptr;
		HANDLE frameFenceEvent = nullptr;
		u64 frameFenceValue = 0;

		std::array<Frame, MaxFramesInFlight> frames;
		u32 frameIndex = 0;
		u32 framesInFlight = 2;
		u32 maxWorkers = 1;

		ID3D12InfoQueue1* infoQueue1 = nullptr;
		DWORD iqCookie = 0;

#if defined(_DEBUG)
		ID3D12Debug* debug = nullptr;
#endif
	};

	Context& GfxContext();

	inline ID3D12CommandQueue* Queue(GfxQueueType type) { return GfxContext().queues[u32(type)]; }
	GfxTextureHandle TexturePoolAlloc();
	DX12Texture& TexturePoolGet(GfxTextureHandle handle);
	void TexturePoolFree(GfxTextureHandle handle);

	GfxBufferHandle BufferPoolAlloc();
	DX12Buffer& BufferPoolGet(GfxBufferHandle handle);
	void BufferPoolFree(GfxBufferHandle handle);

	void CreateStaging(u64 size, ID3D12Resource** resource, D3D12MA::Allocation** allocation);
	void RawTransition(ID3D12GraphicsCommandList* list, ID3D12Resource* resource,
		D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after);

	void CreateGlobalRootSignature();
	GfxPipelineHandle PipelinePoolAlloc();
	DX12Pipeline& PipelinePoolGet(GfxPipelineHandle handle);
	void PipelinePoolFree(GfxPipelineHandle handle);

	u32 BindlessAlloc();
	void BindlessFree(u32 slot);
	D3D12_CPU_DESCRIPTOR_HANDLE BindlessCpu(u32 slot);
	D3D12_GPU_DESCRIPTOR_HANDLE BindlessGpu(u32 slot);

	DXGI_FORMAT ToDXGI(GfxTextureFormat format);
	b8 IsDepthFormat(GfxTextureFormat format);
	DXGI_FORMAT ToTypeless(GfxTextureFormat format);
	DXGI_FORMAT ToShaderViewFormat(GfxTextureFormat format);

	void SetDebugName(ID3D12Object* object, std::string_view name);

	template<typename Fn>
	void ImmediateSubmit(Fn&& record)
	{
		Context& context = GfxContext();

		ID3D12CommandAllocator* alloc = nullptr;
		ID3D12GraphicsCommandList* list = nullptr;
		context.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&alloc));
		context.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc, nullptr, IID_PPV_ARGS(&list));

		record(list);

		list->Close();
		ID3D12CommandList* lists[] = { list };
		context.queues[u32(GfxQueueType::Graphics)]->ExecuteCommandLists(1, lists);

		context.frameFenceValue++;
		context.queues[u32(GfxQueueType::Graphics)]->Signal(context.frameFence, context.frameFenceValue);
		if (context.frameFence->GetCompletedValue() < context.frameFenceValue)
		{
			context.frameFence->SetEventOnCompletion(context.frameFenceValue, context.frameFenceEvent);
			WaitForSingleObjectEx(context.frameFenceEvent, INFINITE, false);
		}

		list->Release();
		alloc->Release();
	}
}