#include "DX12Context.h"

namespace Horizon
{
	u64 GfxFence::GetCompletedValue(GfxFenceHandle handle)
	{
		Context& context = DX12Context();

		u64 value = 0;
		context.fencePool.ResolveRead(handle, [&](const DX12Fence& fence)
			{
				value = fence.pFence->GetCompletedValue();
			});

		return value;
	}

	void GfxFence::WaitForCPU(GfxFenceHandle handle, u64 value)
	{
		Context& context = DX12Context();

		fencePool.ResolveRead(handle, [&](const DX12Fence& fence) 
			{
				if (fence.pFence->GetCompletedValue() >= value)
					return;

				fence.pFence->SetEventOnCompletion(value, fence.pEvent);
				WaitForSingleObject(fence.pEvent, INFINITE);
			});
	}

	void GfxFence::Signal(GfxFenceHandle handle, u64 value)
	{
		Context& context = DX12Context();

		fencePool.ResolveRead(handle, [&](const DX12Fence& fence)
			{
				fence.pFence->Signal(value);
			});
	}
}