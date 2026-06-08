#include "DX12Context.h"

namespace Horizon
{
	GfxCmdListHandle GfxDevice::CreateCommandList(const GfxCommandListDesc& desc)
	{
		Context& context = DX12Context();

		u32 lane = desc.workerIndex * u32(GfxQueueType::Count) + u32(desc.queue);
		u32 local = context.nextLocal[lane]++;
		u32 flat = lane * MaxListsPerWorker + local;

		assert(local < MaxListsPerWorker && "Worker lane ran out of command lists");

		ID3D12CommandAllocator* allocator = context.cmdAllocators[lane * MaxFramesInFlight + context.currentFrameSlot];

		DX12CmdList& cmdList = context.cmdLists[flat];
		cmdList.pAllocator = allocator;
		cmdList.frameSlot = context.currentFrameSlot;

		if (cmdList.pList == nullptr)
		{
			D3D12_COMMAND_LIST_TYPE listType =
				(desc.queue == GfxQueueType::Compute) ? D3D12_COMMAND_LIST_TYPE_COMPUTE :
				(desc.queue == GfxQueueType::Copy) ? D3D12_COMMAND_LIST_TYPE_COPY :
				D3D12_COMMAND_LIST_TYPE_DIRECT;

			HRESULT bResult = context.pDevice->CreateCommandList1(0, listType, D3D12_COMMAND_LIST_FLAG_NONE,
				IID_PPV_ARGS(&cmdList.pList));
			CHECK_HR(bResult, "ID3D12GraphicsCommandList - CreateCommandList1");
		}

		cmdList.pList->Reset(allocator, nullptr);
		cmdList.bRecording = true;

		return GfxCmdListHandle::Generate(flat, 0);
	}

	void GfxCmdList::BeginRendering(GfxCmdListHandle command, const GfxRenderBeginDesc& desc)
	{
		Context& context = DX12Context();
		ID3D12GraphicsCommandList* pList = Helpers::ResolveCmdList(command).pList;

		ID3D12DescriptorHeap* heaps[] = { context.resourceHeap.pHeap };
		pList->SetDescriptorHeaps(1, heaps);
		pList->SetGraphicsRootSignature(context.pGlobalRoot);

		std::array<D3D12_CPU_DESCRIPTOR_HANDLE, 8> rtvHandles{};
		u32 rtvCount = u32(desc.colorTargets.size());

		for (u32 i = 0; i < rtvCount; i++)
		{
			const GfxColorAttachment& attachment = desc.colorTargets[i];
			context.texturePool.ResolveRead(attachment.texture, [&](const DX12Texture& texture)
				{
					rtvHandles[i] = Helpers::CpuAt(context.renderTargetHeap, texture.targetViewIndex);
				});

			if (attachment.loadOp == GfxLoadOp::Clear)
			{
				const f32 color[4] = { attachment.clearColor.r, attachment.clearColor.g, attachment.clearColor.b, attachment.clearColor.a };
				pList->ClearRenderTargetView(rtvHandles[i], color, 0, nullptr);
			}
		}

		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE* pDsvHandle = nullptr;

		if (desc.depth.texture.IsValid())
		{
			context.texturePool.ResolveRead(desc.depth.texture, [&](const DX12Texture& texture)
				{
					dsvHandle = Helpers::CpuAt(context.depthStencilHeap, texture.depthViewIndex);
				});
			pDsvHandle = &dsvHandle;

			if (desc.depth.loadOp == GfxLoadOp::Clear)
			{
				pList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
					desc.depth.clearDepth, desc.depth.clearStencil, 0, nullptr);
			}
		}

		pList->OMSetRenderTargets(rtvCount, rtvHandles.data(), FALSE, pDsvHandle);

		D3D12_VIEWPORT viewport = { 0.0f, 0.0f, f32(desc.width), f32(desc.height), 0.0f, 1.0f };
		D3D12_RECT scissor = { 0, 0, LONG(desc.width), LONG(desc.height) };
		pList->RSSetViewports(1, &viewport);
		pList->RSSetScissorRects(1, &scissor);
	}

	void GfxCmdList::EndRendering(GfxCmdListHandle command)
	{
		DX12CmdList& cmdList = Helpers::ResolveCmdList(command);
		cmdList.pList->Close();
		cmdList.bRecording = false;
	}

	void GfxCmdList::Barrier(GfxCmdListHandle command, std::span<GfxTextureBarrier> barriers)
	{
		Context& context = DX12Context();
		ID3D12GraphicsCommandList* pList = Helpers::ResolveCmdList(command).pList;

		std::array<D3D12_RESOURCE_BARRIER, 16> resourceBarriers{};
		u32 count = 0;

		for (const GfxTextureBarrier& barrier : barriers)
		{
			context.texturePool.ResolveRead(barrier.buffer, [&](const DX12Texture& texture)
				{
					resourceBarriers[count].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					resourceBarriers[count].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
					resourceBarriers[count].Transition.pResource = texture.pResource;
					resourceBarriers[count].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
					resourceBarriers[count].Transition.StateBefore = Helpers::ToResourceState(barrier.before);
					resourceBarriers[count].Transition.StateAfter = Helpers::ToResourceState(barrier.after);
				});
			count++;
		}

		pList->ResourceBarrier(count, resourceBarriers.data());
	}

	void GfxCmdList::Barrier(GfxCmdListHandle command, std::span<GfxBufferBarrier> barriers)
	{
		Context& context = DX12Context();
		ID3D12GraphicsCommandList* pList = Helpers::ResolveCmdList(command).pList;

		std::array<D3D12_RESOURCE_BARRIER, 16> resourceBarriers{};
		u32 count = 0;

		for (const GfxBufferBarrier& barrier : barriers)
		{
			context.bufferPool.ResolveRead(barrier.buffer, [&](const DX12Buffer& buffer)
				{
					resourceBarriers[count].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					resourceBarriers[count].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
					resourceBarriers[count].Transition.pResource = buffer.pResource;
					resourceBarriers[count].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
					resourceBarriers[count].Transition.StateBefore = Helpers::ToResourceState(barrier.before);
					resourceBarriers[count].Transition.StateAfter = Helpers::ToResourceState(barrier.after);
				});
			count++;
		}

		pList->ResourceBarrier(count, resourceBarriers.data());
	}

	void GfxCmdList::UploadBuffer(GfxCmdListHandle command, GfxBufferHandle src, usize srcOffset, GfxBufferHandle dst, usize dstOffset, usize sizeInBytes)
	{
		Context& context = DX12Context();
		ID3D12Resource* pSrc = nullptr;
		ID3D12Resource* pDst = nullptr;

		context.bufferPool.ResolveRead(src, [&](const DX12Buffer& buffer)
			{
				pSrc = buffer.pResource;
			});

		context.bufferPool.ResolveRead(dst, [&](const DX12Buffer& buffer)
			{
				pDst = buffer.pResource;
			});

		Helpers::ResolveCmdList(command).pList->CopyBufferRegion(pDst, dstOffset, pSrc, srcOffset, sizeInBytes);
	}
}