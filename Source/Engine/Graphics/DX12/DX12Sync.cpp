#include "DX12Backend.h"

#include <Engine/Log/Log.h>

#define CHECK_REASON(hr, what) \
	if (FAILED(hr)) { ConsoleLog().Error("{}: {}", what, _com_error(hr).ErrorMessage()); }

namespace Horizon
{
	using namespace DX12;

	GfxSemaphore GfxDevice::CreateSync()
	{
		GfxSemaphore semaphore{};
		semaphore.value = 0;
		ID3D12Fence* fence = nullptr;
		HRESULT hresult = GfxContext().device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
		CHECK_REASON(hresult, "Create Fence/Semaphore");

		semaphore.handle = fence;
		return semaphore;
	}

	void GfxDevice::DestroySemaphore(GfxSemaphore& semaphore)
	{
		if (semaphore.handle)
		{
			((ID3D12Fence*)semaphore.handle)->Release();
			semaphore.handle = nullptr;
		}
	}

	u64 GfxDevice::SemaphoreValue(const GfxSemaphore& semaphore)
	{
		return ((ID3D12Fence*)semaphore.handle)->GetCompletedValue();
	}

	void GfxDevice::WaitSemaphore(const GfxSemaphore& semaphore, u64 value)
	{
		auto* fence = (ID3D12Fence*)semaphore.handle;
		if (fence->GetCompletedValue() >= value)
			return;

		HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		fence->SetEventOnCompletion(value, eventHandle);
		WaitForSingleObjectEx(eventHandle, INFINITE, FALSE);
		CloseHandle(eventHandle);
	}

	void GfxDevice::SignalSemaphore(const GfxSemaphore& semaphore, u64 value)
	{
		((ID3D12Fence*)semaphore.handle)->Signal(value);
	}
}
