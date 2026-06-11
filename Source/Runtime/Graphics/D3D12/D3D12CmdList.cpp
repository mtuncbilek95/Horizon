#include "D3D12Backend.h"

#include <Runtime/Graphics/GfxBackend.h>

namespace Horizon
{
	GfxCmdAllocator* Gfx::CreateGfxCmdAllocator(GfxDevice* pDevice, GfxQueueType type)
	{
		GfxCmdAllocator* pAllocator = new GfxCmdAllocator();
		pAllocator->type = Helpers::ToListType(type);

		HRESULT bResult = pDevice->pDevice->CreateCommandAllocator(pAllocator->type,
			IID_PPV_ARGS(&pAllocator->pAllocator));
		CHECK_HR(bResult, "ID3D12CommandAllocator - CreateCommandAllocator");

		return pAllocator;
	}

	void Gfx::ResetGfxCmdAllocator(GfxCmdAllocator* pAllocator)
	{
		pAllocator->pAllocator->Reset();
	}

	void Gfx::DestroyGfxCmdAllocator(GfxCmdAllocator* pAllocator)
	{
		if (pAllocator->pAllocator)
			pAllocator->pAllocator->Release();

		delete pAllocator;
	}

	GfxCmdList* Gfx::CreateGfxCmdList(GfxDevice* pDevice, GfxCmdAllocator* pAllocator)
	{
		GfxCmdList* pCmd = new GfxCmdList();
		pCmd->type = pAllocator->type;

		HRESULT bResult = pDevice->pDevice->CreateCommandList1(0, pAllocator->type,
			D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&pCmd->pList));
		CHECK_HR(bResult, "ID3D12GraphicsCommandList6 - CreateCommandList1");

		return pCmd;
	}

	void Gfx::BeginGfxCmdList(GfxCmdList* pList, GfxCmdAllocator* pAllocator)
	{
		assert(!pList->bRecording && "Command list is already recording");

		pList->pList->Reset(pAllocator->pAllocator, nullptr);
		pList->bRecording = true;
	}

	void Gfx::CloseGfxCmdList(GfxCmdList* pList)
	{
		assert(pList->bRecording && "Command list is not recording");

		pList->pList->Close();
		pList->bRecording = false;
	}

	void Gfx::DestroyGfxCmdList(GfxCmdList* pList)
	{
		if (pList->pList)
			pList->pList->Release();

		delete pList;
	}

	void Gfx::CmdSetupBindless(GfxCmdList* pCmd, GfxPipelineLayout* pLayout, GfxDescriptorHeap* pResourceHeap)
	{
		assert(pCmd->type != D3D12_COMMAND_LIST_TYPE_COPY && "Copy lists cannot bind root signatures");

		ID3D12DescriptorHeap* heaps[] = { pResourceHeap->pHeap };
		pCmd->pList->SetDescriptorHeaps(1, heaps);

		if (pCmd->type == D3D12_COMMAND_LIST_TYPE_DIRECT)
		{
			pCmd->pList->SetGraphicsRootSignature(pLayout->pLayout);
			pCmd->pList->SetComputeRootSignature(pLayout->pLayout);
		}
		else
		{
			pCmd->pList->SetComputeRootSignature(pLayout->pLayout);
		}
	}

	void Gfx::CmdBeginRendering(GfxCmdList* pCmd, const GfxRenderBeginDesc& desc)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[8] = {};

		for (u32 i = 0; i < desc.colorTargetCount; i++)
		{
			const GfxColorAttachment& attachment = desc.colorTargets[i];
			rtvHandles[i] = attachment.pTexture->targetViewHandle;

			if (attachment.loadOp == GfxLoadOp::Clear)
			{
				const f32 color[4] = { attachment.clearColor.r, attachment.clearColor.g,
									   attachment.clearColor.b, attachment.clearColor.a };
				pCmd->pList->ClearRenderTargetView(rtvHandles[i], color, 0, nullptr);
			}
		}

		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE* pDsvHandle = nullptr;

		if (desc.depth.pTexture != nullptr)
		{
			dsvHandle = desc.depth.pTexture->depthViewHandle;
			pDsvHandle = &dsvHandle;

			if (desc.depth.loadOp == GfxLoadOp::Clear)
			{
				pCmd->pList->ClearDepthStencilView(dsvHandle,
					D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
					desc.depth.clearDepth, desc.depth.clearStencil, 0, nullptr);
			}
		}

		pCmd->pList->OMSetRenderTargets(desc.colorTargetCount, rtvHandles, FALSE, pDsvHandle);

		D3D12_VIEWPORT viewport = { 0.0f, 0.0f, f32(desc.width), f32(desc.height), 0.0f, 1.0f };
		D3D12_RECT scissor = { 0, 0, LONG(desc.width), LONG(desc.height) };
		pCmd->pList->RSSetViewports(1, &viewport);
		pCmd->pList->RSSetScissorRects(1, &scissor);
	}

	void Gfx::CmdBarrier(GfxCmdList* pCmd, const GfxTextureBarrier* pBarriers, u32 count)
	{
		assert(count <= 16 && "Barrier batch limit exceeded");

		D3D12_RESOURCE_BARRIER barriers[16] = {};
		for (u32 i = 0; i < count; i++)
		{
			barriers[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barriers[i].Transition.pResource = pBarriers[i].pTexture->pResource;
			barriers[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barriers[i].Transition.StateBefore = Helpers::ToResourceState(pBarriers[i].before);
			barriers[i].Transition.StateAfter = Helpers::ToResourceState(pBarriers[i].after);
		}

		pCmd->pList->ResourceBarrier(count, barriers);
	}

	void Gfx::CmdBindPipeline(GfxCmdList* pCmd, GfxPipeline* pPipeline)
	{
		pCmd->pList->SetPipelineState(pPipeline->pPipeline);

		if (!pPipeline->bIsCompute && !pPipeline->bUsesMeshShading)
			pCmd->pList->IASetPrimitiveTopology(pPipeline->topology);
	}

	void Gfx::CmdSetGraphicsConstants(GfxCmdList* pCmd, const void* pData, u32 count32Bit, u32 offset32Bit)
	{
		pCmd->pList->SetGraphicsRoot32BitConstants(0, count32Bit, pData, offset32Bit);
	}

	void Gfx::CmdSetComputeConstants(GfxCmdList* pCmd, const void* pData, u32 count32Bit, u32 offset32Bit)
	{
		pCmd->pList->SetComputeRoot32BitConstants(0, count32Bit, pData, offset32Bit);
	}

	void Gfx::CmdBindIndexBuffer(GfxCmdList* pCmd, GfxBuffer* pBuffer)
	{
		D3D12_INDEX_BUFFER_VIEW view = {};
		view.BufferLocation = pBuffer->gpuAddress;
		view.SizeInBytes = u32(pBuffer->sizeInBytes);
		view.Format = pBuffer->stride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;

		pCmd->pList->IASetIndexBuffer(&view);
	}

	void Gfx::CmdDraw(GfxCmdList* pCmd, u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
	{
		pCmd->pList->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void Gfx::CmdDrawIndexed(GfxCmdList* pCmd, u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance)
	{
		pCmd->pList->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void Gfx::CmdDispatch(GfxCmdList* pCmd, u32 groupX, u32 groupY, u32 groupZ)
	{
		pCmd->pList->Dispatch(groupX, groupY, groupZ);
	}

	void Gfx::CmdDispatchMesh(GfxCmdList* pCmd, u32 groupX, u32 groupY, u32 groupZ)
	{
		pCmd->pList->DispatchMesh(groupX, groupY, groupZ);
	}

	void Gfx::CmdCopyBuffer(GfxCmdList* pCmd, GfxBuffer* pSrc, usize srcOffset, GfxBuffer* pDst, usize dstOffset, usize sizeInBytes)
	{
		pCmd->pList->CopyBufferRegion(pDst->pResource, dstOffset, pSrc->pResource, srcOffset, sizeInBytes);
	}

	void Gfx::CmdCopyBufferToTexture(GfxCmdList* pCmd, GfxBuffer* pSrc, usize srcOffset, GfxTexture* pDst, u32 mipLevel, u32 arraySlice)
	{
		const D3D12_RESOURCE_DESC resourceDesc = pDst->pResource->GetDesc();
		const u32 subresource = mipLevel + arraySlice * resourceDesc.MipLevels;

		ID3D12Device* pDevice = nullptr;
		pDst->pResource->GetDevice(IID_PPV_ARGS(&pDevice));

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
		pDevice->GetCopyableFootprints(&resourceDesc, subresource, 1, srcOffset,
			&footprint, nullptr, nullptr, nullptr);
		pDevice->Release();

		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		srcLocation.pResource = pSrc->pResource;
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		srcLocation.PlacedFootprint = footprint;

		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
		dstLocation.pResource = pDst->pResource;
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = subresource;

		pCmd->pList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
	}

	void Gfx::ExecuteGfxCmdLists(GfxQueue* pQueue, GfxCmdList* const* ppLists, u32 count)
	{
		assert(count <= 64 && "Submit batch limit exceeded");

		ID3D12CommandList* lists[64] = {};
		for (u32 i = 0; i < count; i++)
		{
			assert(!ppLists[i]->bRecording && "Cannot submit a command list that is still recording");
			lists[i] = ppLists[i]->pList;
		}

		pQueue->pQueue->ExecuteCommandLists(count, lists);
	}

	u64 Gfx::SignalGfxQueue(GfxQueue* pQueue)
	{
		const u64 value = ++pQueue->value;
		pQueue->pQueue->Signal(pQueue->pTimeline, value);
		return value;
	}

	u64 Gfx::GetGfxQueueCompleted(const GfxQueue* pQueue)
	{
		return pQueue->pTimeline->GetCompletedValue();
	}

	void Gfx::WaitGfxQueueCPU(GfxQueue* pQueue, u64 value)
	{
		if (pQueue->pTimeline->GetCompletedValue() >= value)
			return;

		pQueue->pTimeline->SetEventOnCompletion(value, nullptr);
	}

	void Gfx::WaitGfxQueueGPU(GfxQueue* pWaiter, GfxQueue* pSource, u64 value)
	{
		pWaiter->pQueue->Wait(pSource->pTimeline, value);
	}
}
