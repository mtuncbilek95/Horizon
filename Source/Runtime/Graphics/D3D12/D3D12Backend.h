#pragma once

#include <Runtime/Log/Log.h>
#include <Runtime/Graphics/GfxTypes.h>

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comdef.h>
#include <D3D12MemAlloc.h>

namespace Horizon
{
#define CHECK_HR(hr, what) \
	if(FAILED(hr)) { MainLog::Error("{}: {}", what, _com_error(hr).ErrorMessage()); exit(-1); }

#define CHECK_REASON(hr, what) \
	if(FAILED(hr)) { MainLog::Error("{}: {}", what, _com_error(hr).ErrorMessage()); }

	struct GfxQueue
	{
		ID3D12CommandQueue* pQueue = nullptr;
		ID3D12Fence* pTimeline = nullptr;
		u64 value = 0;

		D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_BUNDLE;
	};

	struct GfxBuffer
	{
		ID3D12Resource* pResource = nullptr;
		D3D12MA::Allocation* pMemory = nullptr;
		void* pMapped = nullptr;

		u64 gpuAddress = 0;
		usize sizeInBytes = 0;
		u32 stride = 0;

		GfxMemoryType memory = GfxMemoryType::GPU;
		u32 shaderViewIndex = ~0u;
		u32 accessViewIndex = ~0u;
	};

	struct GfxTexture
	{
		ID3D12Resource* pResource = nullptr;
		D3D12MA::Allocation* pMemory = nullptr;
		D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;

		GfxTextureType type = GfxTextureType::Tex2D;
		u32 width = 0, height = 0;
		u32 depth = 1, mipLevels = 1;
		u32 targetViewIndex = ~0u;
		u32 depthViewIndex = ~0u;
		u32 shaderViewIndex = ~0u;
		u32 accessViewIndex = ~0u;
		D3D12_CPU_DESCRIPTOR_HANDLE targetViewHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE depthViewHandle = {};
		b8 bIsBackbuffer = false;
	};

	struct GfxPipelineLayout
	{
		ID3D12RootSignature* pLayout = nullptr;
	};

	struct GfxPipeline
	{
		ID3D12PipelineState* pPipeline = nullptr;
		D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		b8 bIsCompute = false;
		b8 bUsesMeshShading = false;
	};

	struct GfxCmdAllocator
	{
		ID3D12CommandAllocator* pAllocator = nullptr;
		D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	};

	struct GfxCmdList
	{
		ID3D12GraphicsCommandList6* pList = nullptr;
		D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		b8 bRecording = false;
	};

	struct GfxDescriptorHeap
	{
		ID3D12DescriptorHeap* pHeap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = {};
		D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = {};
		u32 descriptorSize = 0;
		u32 capacity = 0;

		u32 top = 0;
		std::vector<u32> freeList;
	};

	struct GfxDevice
	{
#if defined(HORIZON_DEBUG)
		ID3D12InfoQueue1* pInfoQueue = nullptr;
		DWORD pInfoId = 0;
#endif

		ID3D12Debug* pDebug = nullptr; // This guy can be active on release mode as well.
		IDXGIFactory7* pFactory = nullptr;
		IDXGIAdapter4* pAdapter = nullptr;
		ID3D12Device10* pDevice = nullptr;
		D3D12MA::Allocator* pAllocator = nullptr;
	};

	struct GfxSwapchain
	{
		IDXGISwapChain4* pSwapchain = nullptr;
		std::vector<GfxTexture> images;

		u32 imageCount = 0;
		b8 vSync = true;
		b8 bAllowTearing = false;
	};

	namespace Helpers
	{
		void CreateTerminalLog(GfxDevice* pContext);

		u32 AllocateDescriptor(GfxDescriptorHeap* pHeap);
		void FreeDescriptor(GfxDescriptorHeap* pHeap, u32 index);

		D3D12_CPU_DESCRIPTOR_HANDLE CpuAt(const GfxDescriptorHeap* pHeap, u32 index);
		D3D12_GPU_DESCRIPTOR_HANDLE GpuAt(const GfxDescriptorHeap* pHeap, u32 index);

		DXGI_FORMAT ToDXGIFormat(GfxTextureFormat format);
		DXGI_FORMAT ToTypelessFormat(DXGI_FORMAT format);
		DXGI_FORMAT ToDepthSRVFormat(DXGI_FORMAT format);
		b8 IsDepthFormat(GfxTextureFormat format);

		D3D12_BLEND ToBlend(GfxBlendFactor factor);
		D3D12_BLEND_OP ToBlendOp(GfxBlendOp op);
		D3D12_COMPARISON_FUNC ToCompare(GfxCompareOp op);
		D3D12_PRIMITIVE_TOPOLOGY_TYPE ToTopologyType(GfxPrimitiveTopology topology);
		D3D_PRIMITIVE_TOPOLOGY ToTopology(GfxPrimitiveTopology topology);
		D3D12_RESOURCE_STATES ToResourceState(GfxResourceState state);
		D3D12_COMMAND_LIST_TYPE ToListType(GfxQueueType type);
	}
}