#include "D3D12Queue.h"

#include <Runtime/Log/Terminal.h>
#include <Runtime/D3D12/D3D12CommandList.h>
#include <Runtime/D3D12/D3D12Device.h>
#include <Runtime/D3D12/D3D12Fence.h>

namespace Horizon::RHI
{
	D3D12Queue::~D3D12Queue()
	{
		if (m_ownerDevice)
			static_cast<D3D12Device*>(m_ownerDevice)->ForgetQueue(this);

		if (m_queue)
			m_queue->Release();
	}

	void D3D12Queue::Submit(GfxCommandList* const* ppLists, u32 count)
	{
		if (count > 64)
		{
			Terminal::Error("D3D12Queue", "Submit batch limit exceeded, {} > 64", count);
			return;
		}

		ID3D12CommandList* native[64];

		for (u32 i = 0; i < count; i++)
			native[i] = static_cast<D3D12CommandList*>(ppLists[i])->Handle();

		m_queue->ExecuteCommandLists(count, native);
	}

	u64 D3D12Queue::Signal(GfxFence* pFence)
	{
		auto* pD3DFence = static_cast<D3D12Fence*>(pFence);

		const u64 signalValue = pD3DFence->Advance();

		m_queue->Signal(pD3DFence->Handle(), signalValue);
		return signalValue;
	}

	void D3D12Queue::Wait(GfxFence* pFence, u64 value)
	{
		auto* pD3DFence = static_cast<D3D12Fence*>(pFence);

		m_queue->Wait(pD3DFence->Handle(), value);
	}
}