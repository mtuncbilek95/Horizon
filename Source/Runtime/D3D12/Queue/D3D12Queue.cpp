#include "D3D12Queue.h"

#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>
/*#include <Runtime/D3D12/D3D12CommandList.h>
#include <Runtime/D3D12/D3D12Device.h>
#include <Runtime/D3D12/D3D12Fence.h>*/

namespace Horizon::RHI
{
	D3D12Queue::~D3D12Queue()
	{
		if (m_queue)
			m_queue->Release();
	}

	void D3D12Queue::Submit(List<CommandList*> cmdlists)
	{
		if (cmdlists.GetCount() > 64)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Submit batch limit exceeded, {} > 64", cmdlists.GetCount());
			return;
		}

		ID3D12CommandList* native[64];

		/*for (u32 i = 0; i < cmdlists.GetCount(); i++)
			native[i] = static_cast<D3D12CommandList*>(ppLists[i])->Handle();*/

		m_queue->ExecuteCommandLists(cmdlists.GetCount(), native);
	}

	u64 D3D12Queue::Signal(Fence* pFence)
	{
		auto* pD3DFence = static_cast<D3D12Fence*>(pFence);

		const u64 signalValue = pD3DFence->Advance();

		m_queue->Signal(pD3DFence->Handle(), signalValue);
		return signalValue;
	}

	void D3D12Queue::Wait(Fence* pFence, u64 value)
	{

	}

	u64 D3D12Queue::GetTimestampFrequency()
	{
	}

	void D3D12Queue::SetDebugName(const char* pName)
	{
	}
}