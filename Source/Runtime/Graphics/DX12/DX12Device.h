#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>
#include <comdef.h>
#include <D3D12MemAlloc.h>

#include <array>
#include <vector>
#include <functional>
#include <mutex>
#include <memory>

namespace Horizon
{
	class GfxCommandList;

	inline constexpr u64 MaxBindless = 1 << 16;

	struct DX12PendingDelete
	{
		u64 value = 0;
		std::function<void()> fn;
	};

	struct DX12WorkerCmd
	{
		ID3D12CommandAllocator* allocator = nullptr;   // DIRECT
		std::vector<std::unique_ptr<GfxCommandList>> lists;
		u32 nextList = 0;
	};

	struct DX12Frame
	{
		std::vector<DX12WorkerCmd> workers;
		u64 fenceValue = 0;
	};

	struct DX12DescriptorHeap
	{
		ID3D12DescriptorHeap* heap = nullptr;
		u32 stride = 0;
		u32 capacity = 0;
		u32 nextFree = 0;
		std::vector<u32> freeList;

		u32 Allocate()
		{
			if (!freeList.empty())
			{
				u32 val = freeList.back();
				freeList.pop_back();
				return val;
			}

			return nextFree++;
		}

		void Free(u32 slot)
		{
			freeList.push_back(slot);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(u32 slot) const
		{
			auto cpuHeap = heap->GetCPUDescriptorHandleForHeapStart();
			cpuHeap.ptr += usize(slot) * stride;
			return cpuHeap;
		}

		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(u32 slot) const
		{
			auto gpuHeap = heap->GetGPUDescriptorHandleForHeapStart();
			gpuHeap.ptr += usize(slot) * stride;
			return gpuHeap;
		}
	};

	struct DX12Device
	{
#if defined(_DEBUG)
		ID3D12Debug* debug = nullptr;
		ID3D12InfoQueue1* infoQueue1 = nullptr;
		DWORD iqCookie = 0;
#endif

		IDXGIFactory7* factory = nullptr;
		IDXGIAdapter4* adapter = nullptr;
		ID3D12Device10* device = nullptr;
		D3D12MA::Allocator* allocator = nullptr;

		std::array<ID3D12CommandQueue*, 3> queues;

		DX12DescriptorHeap bindless;
		DX12DescriptorHeap rtv;
		DX12DescriptorHeap dsv;

		ID3D12Fence* frameFence = nullptr;
		HANDLE frameFenceEvent = nullptr;
		u64 frameValue = 0;

		ID3D12RootSignature* globalRootSignature = nullptr;

		u32 framesInFlight = 2;
		u32 maxWorkers = 1;
		u32 frameIndex = 0;
		std::vector<DX12Frame> frames;

		std::vector<DX12PendingDelete> pendingDeletes;
		std::mutex deleteMutex;
	};
}