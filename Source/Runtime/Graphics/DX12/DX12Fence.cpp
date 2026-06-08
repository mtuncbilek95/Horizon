#include "DX12Context.h"

namespace Horizon
{
	GfxFenceHandle GfxDevice::CreateFence(const GfxFenceDesc& desc)
	{
		Context& context = DX12Context();

		DX12Fence fence = {};
		HRESULT bResult = context.pDevice->CreateFence(desc.initialValue, D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&fence.pFence));
		CHECK_HR(bResult, "ID3D12Fence - CreateFence");

		fence.pEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		fence.fenceValue = desc.initialValue;

		return context.fencePool.Insert(std::move(fence));
	}

	void GfxDevice::DestroyFence(GfxFenceHandle handle)
	{
		Context& context = DX12Context();

		context.fencePool.ResolveWrite(handle, [&](DX12Fence& fence)
			{
				if (fence.pEvent)
				{
					CloseHandle(fence.pEvent);
				}

				if (fence.pFence)
				{
					fence.pFence->Release();
				}
			});

		context.fencePool.Remove(handle);
	}

	u64 GfxFence::ReserveValue(GfxFenceHandle handle)
	{
		Context& context = DX12Context();

		u64 value = 0;
		context.fencePool.ResolveWrite(handle, [&](DX12Fence& fence)
			{
				value = ++fence.fenceValue;
			});

		return value;
	}

	u64 GfxFence::GetCompletedValue(GfxFenceHandle handle)
	{
		Context& context = DX12Context();

		u64 value = 0;
		context.fencePool.ResolveRead(handle, [&](const DX12Fence& fItem)
			{
				value = fItem.pFence->GetCompletedValue();
			});

		return value;
	}

	void GfxFence::WaitForCPU(GfxFenceHandle handle, u64 value)
	{
		Context& context = DX12Context();

		ID3D12Fence* pFence = nullptr;
		HANDLE pEvent = nullptr;

		context.fencePool.ResolveRead(handle, [&](const DX12Fence& fItem)
			{
				pFence = fItem.pFence;
				pEvent = fItem.pEvent;
			});

		if (pFence == nullptr)
		{
			return;
		}

		if (pFence->GetCompletedValue() >= value)
		{
			return;
		}

		pFence->SetEventOnCompletion(value, pEvent);
		WaitForSingleObject(pEvent, INFINITE);
	}

	void GfxFence::Signal(GfxFenceHandle handle, u64 value)
	{
		Context& context = DX12Context();

		context.fencePool.ResolveRead(handle, [&](const DX12Fence& fItem)
			{
				fItem.pFence->Signal(value);
			});
	}
}