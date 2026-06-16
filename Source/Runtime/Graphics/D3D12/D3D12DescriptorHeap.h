#pragma once

#include <Runtime/Graphics/D3D12/D3D12Backend.h>

namespace Horizon
{
	struct D3D12DescriptorHeap
	{
		ID3D12DescriptorHeap* pHeap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE cpuStart{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpuStart{};
		u32 descriptorSize = 0;
		u32 capacity = 0;
		u32 top = 0;
		std::vector<u32> freeList;

		u32 Allocate()
		{
			if (!freeList.empty())
			{
				u32 idx = freeList.back();
				freeList.pop_back();
				return idx;
			}

			assert(top < capacity && "Heap is full");
			return top++;
		}

		void Free(u32 index) { freeList.push_back(index); }

		D3D12_CPU_DESCRIPTOR_HANDLE CpuAt(u32 i) const { return { cpuStart.ptr + usize(i) * descriptorSize }; }
	};
}