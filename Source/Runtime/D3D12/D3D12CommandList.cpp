#include "D3D12CommandList.h"

#include <Runtime/Containers/StringOps.h>
#include <Runtime/Log/Terminal.h>

#include <Runtime/D3D12/D3D12Buffer.h>
#include <Runtime/D3D12/D3D12DescriptorHeap.h>
#include <Runtime/D3D12/D3D12Device.h>
#include <Runtime/D3D12/D3D12Pipeline.h>
#include <Runtime/D3D12/D3D12Texture.h>

namespace Horizon::RHI
{
	namespace
	{
		constexpr u32 kMaxBarrierBatch = 32;
	}

	D3D12CommandList::~D3D12CommandList()
	{
		if (m_list)
			m_list->Release();

		if (m_allocator)
			m_allocator->Release();
	}

	void D3D12CommandList::Begin()
	{
		m_allocator->Reset();
		m_list->Reset(m_allocator, nullptr);

		m_rendering = false;
	}

	void D3D12CommandList::End()
	{
		if (m_rendering)
			EndRendering();

		m_list->Close();
	}

	void D3D12CommandList::BindDescriptorHeaps(GfxDescriptorHeap* pResourceHeap, GfxDescriptorHeap* pSamplerHeap)
	{
		if (m_type == D3D12_COMMAND_LIST_TYPE_COPY)
		{
			Terminal::Error(StringOps::GetName(this), "Copy lists cannot bind descriptor heaps");
			return;
		}

		ID3D12DescriptorHeap* heaps[2] = {};
		u32 count = 0;

		if (pResourceHeap)
			heaps[count++] = static_cast<D3D12DescriptorHeap*>(pResourceHeap)->Handle();

		if (pSamplerHeap)
			heaps[count++] = static_cast<D3D12DescriptorHeap*>(pSamplerHeap)->Handle();

		if (count > 0)
			m_list->SetDescriptorHeaps(count, heaps);

		ID3D12RootSignature* pRootSignature = m_device->RootSignature();

		m_list->SetComputeRootSignature(pRootSignature);

		if (m_type == D3D12_COMMAND_LIST_TYPE_DIRECT)
			m_list->SetGraphicsRootSignature(pRootSignature);
	}

	void D3D12CommandList::Barrier(const GfxTextureBarrier* pBarriers, u32 count)
	{
		if (count > kMaxBarrierBatch)
		{
			Terminal::Error(StringOps::GetName(this), "Barrier batch limit exceeded, {} > {}", count, kMaxBarrierBatch);
			return;
		}

		D3D12_RESOURCE_BARRIER native[kMaxBarrierBatch] = {};

		for (u32 i = 0; i < count; i++)
		{
			auto* pTexture = static_cast<D3D12Texture*>(pBarriers[i].pTexture);

			native[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			native[i].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			native[i].Transition.pResource = pTexture->Handle();
			native[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			native[i].Transition.StateBefore = Helpers::ToResourceState(pBarriers[i].before);
			native[i].Transition.StateAfter = Helpers::ToResourceState(pBarriers[i].after);
		}

		m_list->ResourceBarrier(count, native);
	}

	void D3D12CommandList::Barrier(const GfxBufferBarrier* pBarriers, u32 count)
	{
		if (count > kMaxBarrierBatch)
		{
			Terminal::Error(StringOps::GetName(this), "Barrier batch limit exceeded, {} > {}", count, kMaxBarrierBatch);
			return;
		}

		D3D12_RESOURCE_BARRIER native[kMaxBarrierBatch] = {};

		for (u32 i = 0; i < count; i++)
		{
			auto* pBuffer = static_cast<D3D12Buffer*>(pBarriers[i].pBuffer);

			native[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			native[i].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			native[i].Transition.pResource = pBuffer->Handle();
			native[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			native[i].Transition.StateBefore = Helpers::ToResourceState(pBarriers[i].before);
			native[i].Transition.StateAfter = Helpers::ToResourceState(pBarriers[i].after);
		}

		m_list->ResourceBarrier(count, native);
	}

	void D3D12CommandList::BarrierUav()
	{
		D3D12_RESOURCE_BARRIER barrier = {};

		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.UAV.pResource = nullptr;

		m_list->ResourceBarrier(1, &barrier);
	}

	void D3D12CommandList::BeginRendering(const GfxRenderBeginDesc& desc)
	{
		D3D12_RENDER_PASS_RENDER_TARGET_DESC colorTargets[8] = {};

		for (u32 i = 0; i < desc.colorTargetCount; i++)
		{
			const GfxColorAttachment& attachment = desc.colorTargets[i];
			auto* pTexture = static_cast<D3D12Texture*>(attachment.pTexture);

			colorTargets[i].cpuDescriptor = pTexture->RenderTargetHandle();
			colorTargets[i].BeginningAccess.Type = Helpers::ToBeginAccess(attachment.loadOp);
			colorTargets[i].BeginningAccess.Clear.ClearValue.Format = pTexture->Format();
			colorTargets[i].BeginningAccess.Clear.ClearValue.Color[0] = attachment.clearColor.r;
			colorTargets[i].BeginningAccess.Clear.ClearValue.Color[1] = attachment.clearColor.g;
			colorTargets[i].BeginningAccess.Clear.ClearValue.Color[2] = attachment.clearColor.b;
			colorTargets[i].BeginningAccess.Clear.ClearValue.Color[3] = attachment.clearColor.a;
			colorTargets[i].EndingAccess.Type = Helpers::ToEndAccess(attachment.storeOp);
		}

		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthTarget = {};
		const b8 bHasDepth = desc.depth.pTexture != nullptr;

		if (bHasDepth)
		{
			auto* pTexture = static_cast<D3D12Texture*>(desc.depth.pTexture);

			depthTarget.cpuDescriptor = pTexture->DepthStencilHandle();
			depthTarget.DepthBeginningAccess.Type = Helpers::ToBeginAccess(desc.depth.loadOp);
			depthTarget.DepthBeginningAccess.Clear.ClearValue.Format = pTexture->Format();
			depthTarget.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = desc.depth.clearDepth;
			depthTarget.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = desc.depth.clearStencil;
			depthTarget.DepthEndingAccess.Type = Helpers::ToEndAccess(desc.depth.storeOp);
			depthTarget.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
			depthTarget.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
		}

		m_list->BeginRenderPass(desc.colorTargetCount, colorTargets,
			bHasDepth ? &depthTarget : nullptr, D3D12_RENDER_PASS_FLAG_NONE);

		m_rendering = true;

		const GfxViewport viewport = { 0.0f, 0.0f, f32(desc.width), f32(desc.height), 0.0f, 1.0f };
		const GfxScissor scissor = { 0, 0, i32(desc.width), i32(desc.height) };

		SetViewport(viewport);
		SetScissor(scissor);
	}

	void D3D12CommandList::EndRendering()
	{
		if (!m_rendering)
			return;

		m_list->EndRenderPass();
		m_rendering = false;
	}

	void D3D12CommandList::BindPipeline(GfxPipeline* pPipeline)
	{
		auto* pD3DPipeline = static_cast<D3D12Pipeline*>(pPipeline);

		m_list->SetPipelineState(pD3DPipeline->GetPipeline());

		if (pD3DPipeline->GetType() == GfxPipelineType::Graphics && !pD3DPipeline->UsesMeshShading())
			m_list->IASetPrimitiveTopology(pD3DPipeline->GetTopology());
	}

	void D3D12CommandList::SetGraphicsConstants(const void* pData, u32 count32, u32 offset32)
	{
		m_list->SetGraphicsRoot32BitConstants(0, count32, pData, offset32);
	}

	void D3D12CommandList::SetComputeConstants(const void* pData, u32 count32, u32 offset32)
	{
		m_list->SetComputeRoot32BitConstants(0, count32, pData, offset32);
	}

	void D3D12CommandList::SetViewports(const GfxViewport* pViewports, u32 count)
	{
		D3D12_VIEWPORT native[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};

		if (count > D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)
		{
			Terminal::Error(StringOps::GetName(this), "Viewport count exceeded, {} > {}", count,
				u32(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));
			return;
		}

		for (u32 i = 0; i < count; i++)
		{
			native[i].TopLeftX = pViewports[i].x;
			native[i].TopLeftY = pViewports[i].y;
			native[i].Width = pViewports[i].width;
			native[i].Height = pViewports[i].height;
			native[i].MinDepth = pViewports[i].minDepth;
			native[i].MaxDepth = pViewports[i].maxDepth;
		}

		m_list->RSSetViewports(count, native);
	}

	void D3D12CommandList::SetScissors(const GfxScissor* pScissors, u32 count)
	{
		D3D12_RECT native[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE] = {};

		if (count > D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE)
		{
			Terminal::Error(StringOps::GetName(this), "Scissor count exceeded, {} > {}", count,
				u32(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));
			return;
		}

		for (u32 i = 0; i < count; i++)
		{
			native[i].left = pScissors[i].x;
			native[i].top = pScissors[i].y;
			native[i].right = pScissors[i].x + pScissors[i].width;
			native[i].bottom = pScissors[i].y + pScissors[i].height;
		}

		m_list->RSSetScissorRects(count, native);
	}

	void D3D12CommandList::BindIndexBuffer(GfxBuffer* pBuffer, GfxIndexType type)
	{
		auto* pD3DBuffer = static_cast<D3D12Buffer*>(pBuffer);

		D3D12_INDEX_BUFFER_VIEW view = {};

		view.BufferLocation = pD3DBuffer->Handle()->GetGPUVirtualAddress();
		view.SizeInBytes = u32(pD3DBuffer->GetDesc().size);
		view.Format = Helpers::ToIndexFormat(type);

		m_list->IASetIndexBuffer(&view);
	}

	void D3D12CommandList::Draw(u32 vtxCount, u32 instCount, u32 firstVtx, u32 firstInst)
	{
		m_list->DrawInstanced(vtxCount, instCount, firstVtx, firstInst);
	}

	void D3D12CommandList::DrawIndexed(u32 idxCount, u32 instCount, u32 firstIdx, i32 vtxOffset, u32 firstInst)
	{
		m_list->DrawIndexedInstanced(idxCount, instCount, firstIdx, vtxOffset, firstInst);
	}

	void D3D12CommandList::DrawIndirect(GfxBuffer* pArgs, usize offset, u32 drawCount)
	{
		auto* pD3DBuffer = static_cast<D3D12Buffer*>(pArgs);

		m_list->ExecuteIndirect(m_device->DrawSignature(), drawCount, pD3DBuffer->Handle(), offset, nullptr, 0);
	}

	void D3D12CommandList::DrawIndexedIndirect(GfxBuffer* pArgs, usize offset, u32 drawCount)
	{
		auto* pD3DBuffer = static_cast<D3D12Buffer*>(pArgs);

		m_list->ExecuteIndirect(m_device->DrawIndexedSignature(), drawCount, pD3DBuffer->Handle(), offset, nullptr, 0);
	}

	void D3D12CommandList::Dispatch(u32 groupX, u32 groupY, u32 groupZ)
	{
		m_list->Dispatch(groupX, groupY, groupZ);
	}

	void D3D12CommandList::DispatchMesh(u32 groupX, u32 groupY, u32 groupZ)
	{
		m_list->DispatchMesh(groupX, groupY, groupZ);
	}

	void D3D12CommandList::DispatchIndirect(GfxBuffer* pArgs, usize offset)
	{
		auto* pD3DBuffer = static_cast<D3D12Buffer*>(pArgs);

		m_list->ExecuteIndirect(m_device->DispatchSignature(), 1, pD3DBuffer->Handle(), offset, nullptr, 0);
	}

	void D3D12CommandList::CopyBuffer(GfxBuffer* pSrc, usize srcOff, GfxBuffer* pDst, usize dstOff, usize size)
	{
		auto* pD3DSrc = static_cast<D3D12Buffer*>(pSrc);
		auto* pD3DDst = static_cast<D3D12Buffer*>(pDst);

		m_list->CopyBufferRegion(pD3DDst->Handle(), dstOff, pD3DSrc->Handle(), srcOff, size);
	}

	void D3D12CommandList::CopyBufferToTexture(GfxBuffer* pSrc, usize srcOff, GfxTexture* pDst, u32 mipLevel, u32 arraySlice)
	{
		auto* pD3DSrc = static_cast<D3D12Buffer*>(pSrc);
		auto* pD3DDst = static_cast<D3D12Texture*>(pDst);

		const D3D12_RESOURCE_DESC resourceDesc = pD3DDst->Handle()->GetDesc();
		const u32 subresource = mipLevel + arraySlice * pD3DDst->GetDesc().mipLevels;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};

		m_device->Handle()->GetCopyableFootprints(&resourceDesc, subresource, 1, srcOff, &footprint, nullptr, nullptr, nullptr);

		D3D12_TEXTURE_COPY_LOCATION source = {};

		source.pResource = pD3DSrc->Handle();
		source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		source.PlacedFootprint = footprint;

		D3D12_TEXTURE_COPY_LOCATION destination = {};

		destination.pResource = pD3DDst->Handle();
		destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		destination.SubresourceIndex = subresource;

		m_list->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
	}

	void D3D12CommandList::CopyTextureToBuffer(GfxTexture* pSrc, u32 mipLevel, u32 arraySlice, GfxBuffer* pDst, usize dstOff)
	{
		auto* pD3DSrc = static_cast<D3D12Texture*>(pSrc);
		auto* pD3DDst = static_cast<D3D12Buffer*>(pDst);

		const D3D12_RESOURCE_DESC resourceDesc = pD3DSrc->Handle()->GetDesc();
		const u32 subresource = mipLevel + arraySlice * pD3DSrc->GetDesc().mipLevels;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};

		m_device->Handle()->GetCopyableFootprints(&resourceDesc, subresource, 1, dstOff, &footprint, nullptr, nullptr, nullptr);

		D3D12_TEXTURE_COPY_LOCATION source = {};

		source.pResource = pD3DSrc->Handle();
		source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		source.SubresourceIndex = subresource;

		D3D12_TEXTURE_COPY_LOCATION destination = {};

		destination.pResource = pD3DDst->Handle();
		destination.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		destination.PlacedFootprint = footprint;

		m_list->CopyTextureRegion(&destination, 0, 0, 0, &source, nullptr);
	}
}
