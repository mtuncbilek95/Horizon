#include "D3D12CpuDescriptorPool.h"

#include <Runtime/Containers/ScopedLock.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

namespace Horizon::RHI
{
	D3D12CpuDescriptorPool::~D3D12CpuDescriptorPool()
	{
		Shutdown();
	}

	void D3D12CpuDescriptorPool::Shutdown()
	{
		if (m_heap)
		{
			m_heap->Release();
			m_heap = nullptr;
		}
	}

	HRESULT D3D12CpuDescriptorPool::Initialize(ID3D12Device10* pDevice, D3D12_DESCRIPTOR_HEAP_TYPE type, u32 capacity)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = type;
		heapDesc.NumDescriptors = capacity;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		HRESULT hr = pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_heap));
		if (FAILED(hr))
			return hr;

		m_start = m_heap->GetCPUDescriptorHandleForHeapStart();
		m_increment = pDevice->GetDescriptorHandleIncrementSize(type);
		m_capacity = capacity;
		m_next = 0;
		m_freeIndices.Reserve(capacity);

		return S_OK;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE D3D12CpuDescriptorPool::Allocate()
	{
		ScopedLock lock(m_lock);

		u32 index = 0;
		if (!m_freeIndices.IsEmpty())
		{
			index = m_freeIndices.Back();
			m_freeIndices.PopBack();
		}
		else if (m_next < m_capacity)
		{
			index = m_next++;
		}
		else
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Pool is full, capacity: {}", m_capacity);
			return {};
		}

		D3D12_CPU_DESCRIPTOR_HANDLE handle = {};
		handle.ptr = m_start.ptr + static_cast<SIZE_T>(index) * m_increment;
		return handle;
	}

	void D3D12CpuDescriptorPool::Free(D3D12_CPU_DESCRIPTOR_HANDLE handle)
	{
		if (handle.ptr == 0)
			return;

		u32 index = static_cast<u32>((handle.ptr - m_start.ptr) / m_increment);

		ScopedLock lock(m_lock);
		m_freeIndices.PushBack(index);
	}
}