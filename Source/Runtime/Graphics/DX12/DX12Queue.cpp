#include "DX12Context.h"

#include <array>

namespace Horizon
{
	void GfxQueue::ExecuteCommandLists(GfxQueueType type, std::span<GfxCmdListHandle> handles)
	{
		Context& context = DX12Context();

		std::array<ID3D12CommandList*, 64> cmds;
		for (usize i = 0; i < handles.size(); i++)
			cmds[i] = context.cmdLists[handles[i].Index()].pList;

		context.queuePool[u32(type)]->ExecuteCommandLists(u32(handles.size()), cmds.data());
	}

	void GfxQueue::Signal(GfxQueueType type, GfxFenceHandle handle, u64 value)
	{
		Context& context = DX12Context();
		context.fencePool.ResolveRead(handle, [&](const DX12Fence& fence)
			{
				context.queuePool[u32(type)]->Signal(fence.pFence, value);
			});
	}

	void GfxQueue::Wait(GfxQueueType type, GfxFenceHandle handle, u64 value)
	{
		Context& context = DX12Context();
		context.fencePool.ResolveRead(handle, [&](const DX12Fence& fence)
			{
				context.queuePool[u32(type)]->Wait(fence.pFence, value);
			});
	}
}