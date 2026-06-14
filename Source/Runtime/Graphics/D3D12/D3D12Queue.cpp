#include "D3D12Queue.h"

#include <Runtime/Graphics/D3D12/D3D12Fence.h>
#include <Runtime/Graphics/D3D12/D3D12CommandList.h>

#include <cassert>

namespace Horizon
{
	D3D12Queue::~D3D12Queue() 
	{ 
		if (m_queue) 
			m_queue->Release(); 
	}

	void D3D12Queue::Submit(GfxCommandList* const* lists, u32 count)
	{
		assert(count <= 64 && "Submit batch limit");

		ID3D12CommandList* native[64];
		for (u32 i = 0; i < count; i++)
			native[i] = static_cast<D3D12CommandList*>(lists[i])->Handle();
		m_queue->ExecuteCommandLists(count, native);
	}

	u64 D3D12Queue::Signal(GfxFence* fence)
	{
		auto* d3d12Fence = static_cast<D3D12Fence*>(fence);
		u64 signalValue = d3d12Fence->Advance();
		m_queue->Signal(d3d12Fence->Handle(), signalValue);
		return signalValue;
	}

	void D3D12Queue::Wait(GfxFence* fence, u64 value)
	{
		auto* d3d12Fence = static_cast<D3D12Fence*>(fence);
		m_queue->Wait(d3d12Fence->Handle(), value);
	}
}