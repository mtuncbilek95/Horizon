#include "D3D12Backend.h"

#include <Runtime/Graphics/GfxBackend.h>

namespace Horizon
{
	GfxQueue* Gfx::CreateGfxQueue(GfxDevice* pContext, const GfxQueueDesc& desc)
	{
		GfxQueue* pQueue = new GfxQueue();

		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = desc.type == GfxQueueType::Graphics ? D3D12_COMMAND_LIST_TYPE_DIRECT :
			desc.type == GfxQueueType::Compute ? D3D12_COMMAND_LIST_TYPE_COMPUTE : D3D12_COMMAND_LIST_TYPE_COPY;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		HRESULT bResult = pContext->pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&pQueue->pQueue));
		CHECK_HR(bResult, "ID3D12CommandQueue - CreateCommandQueue");

		bResult = pContext->pDevice->CreateFence(pQueue->value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pQueue->pTimeline));
		CHECK_HR(bResult, "ID3D12Fence - CreateFence");

		pQueue->type = queueDesc.Type;

		return pQueue;
	}

	void Gfx::DestroyGfxQueue(GfxQueue* qHandle)
	{
		if (qHandle->pQueue && qHandle->pTimeline && qHandle->pTimeline->GetCompletedValue() < qHandle->value)
			qHandle->pTimeline->SetEventOnCompletion(qHandle->value, nullptr);

		if (qHandle->pTimeline)
			qHandle->pTimeline->Release();

		if (qHandle->pQueue)
			qHandle->pQueue->Release();

		delete qHandle;
	}
}