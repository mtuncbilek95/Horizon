#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/PAL/Sync/Mutex.h>

#include <dxgi1_6.h>
#include <d3d12.h>

namespace Horizon::RHI
{
	class D3D12CpuDescriptorPool final
	{
	public:
		~D3D12CpuDescriptorPool();

		HRESULT Initialize(ID3D12Device10* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 capacity);
		void Shutdown();

		D3D12_CPU_DESCRIPTOR_HANDLE Allocate();
		void Free(D3D12_CPU_DESCRIPTOR_HANDLE handle);

	private:
		ID3D12DescriptorHeap* m_heap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE m_start = {};

		u32 m_increment = 0;
		u32 m_capacity = 0;
		u32 m_next = 0;

		List<u32> m_freeIndices;
		PAL::Mutex m_lock;
	};
}