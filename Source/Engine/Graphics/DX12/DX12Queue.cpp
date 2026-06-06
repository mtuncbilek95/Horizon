#include "DX12Backend.h"

#include <Engine/Graphics/RHI/GfxQueue.h>

namespace Horizon
{
	void GfxQueue::Submit(GfxQueueType type, const GfxSubmitInfo& info)
	{
		using namespace DX12;
		ID3D12CommandQueue* queue = Queue(type);

		for (const GfxWaitInfo& wait : info.waits)
			queue->Wait((ID3D12Fence*)wait.semaphore.handle, wait.value);

		ID3D12CommandList* native[64];
		u32 listCount = 0;
		for (GfxCommandList* cmdList : info.lists)
		{
			cmdList->list->Close();
			native[listCount++] = cmdList->list;
		}

		if (listCount)
			queue->ExecuteCommandLists(listCount, native);

		for (const GfxSignalInfo& signal : info.signals)
			queue->Signal((ID3D12Fence*)signal.semaphore.handle, signal.value);
	}
}
