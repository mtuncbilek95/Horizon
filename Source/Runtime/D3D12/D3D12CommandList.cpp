#include "D3D12CommandList.h"

#include <Runtime/D3D12/D3D12Buffer.h>
#include <Runtime/D3D12/D3D12DescriptorHeap.h>
#include <Runtime/D3D12/D3D12Device.h>
#include <Runtime/D3D12/D3D12Pipeline.h>
#include <Runtime/D3D12/D3D12Texture.h>

namespace Horizon
{
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
	}

	void D3D12CommandList::End()
	{
		m_list->Close();
	}

	void D3D12CommandList::SetupBindless()
	{
		if (m_type == D3D12_COMMAND_LIST_TYPE_COPY)
			return;

		auto* pResourceHeap = static_cast<D3D12DescriptorHeap*>(m_device->GetDescriptorHeap(GfxDescriptorHeapType::Resource));
		auto* pSamplerHeap = static_cast<D3D12DescriptorHeap*>(m_device->GetDescriptorHeap(GfxDescriptorHeapType::Sampler));

		ID3D12DescriptorHeap* heaps[] = { pResourceHeap->Handle(), pSamplerHeap->Handle() };

		m_list->SetDescriptorHeaps(2, heaps);

		ID3D12RootSignature* pRootSig = m_device->GetRootSignature();

		if (m_type == D3D12_COMMAND_LIST_TYPE_DIRECT)
		{
			m_list->SetGraphicsRootSignature(pRootSig);
			m_list->SetComputeRootSignature(pRootSig);
		}
		else
		{
			m_list->SetComputeRootSignature(pRootSig);
		}
	}

	void D3D12CommandList::Barrier(const GfxTextureBarrier* pBarriers, u32 count)
	{
		if (count > 16)
		{
			Terminal::Error("D3D12CommandList", "Texture barrier batch limit exceeded, {} > 16", count);
			return;
		}

		D3D12_RESOURCE_BARRIER native[16] = {};

		for (u32 i = 0; i < count; i++)
		{
			native[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			native[i].Transition.pResource = static_cast<D3D12Texture*>(pBarriers[i].pTexture)->GetResource();
			native[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			native[i].Transition.StateBefore = Helpers::ToResourceState(pBarriers[i].before);
			native[i].Transition.StateAfter = Helpers::ToResourceState(pBarriers[i].after);
		}

		m_list->ResourceBarrier(count, native);
	}

	void D3D12CommandList::Barrier(const GfxBufferBarrier* pBarriers, u32 count)
	{
		if (count > 16)
		{
			Terminal::Error("D3D12CommandList", "Buffer barrier batch limit exceeded, {} > 16", count);
			return;
		}

		D3D12_RESOURCE_BARRIER native[16] = {};

		for (u32 i = 0; i < count; i++)
		{
			native[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			native[i].Transition.pResource = static_cast<D3D12Buffer*>(pBarriers[i].pBuffer)->GetResource();
			native[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			native[i].Transition.StateBefore = Helpers::ToResourceState(pBarriers[i].before);
			native[i].Transition.StateAfter = Helpers::ToResourceState(pBarriers[i].after);
		}

		m_list->ResourceBarrier(count, native);
	}

	void D3D12CommandList::BarrierUav()
	{
		D3D12_RESOURCE_BARRIER native = {};

		native.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		native.UAV.pResource = nullptr;

		m_list->ResourceBarrier(1, &native);
	}

	void D3D12CommandList::BeginRendering(const GfxRenderBeginDesc& desc)
	{
		D3D12_RENDER_PASS_RENDER_TARGET_DESC targets[8] = {};

		for (u32 i = 0; i < desc.colorTargetCount; i++)
		{
			const GfxColorAttachment& attachment = desc.colorTargets[i];
			auto* pTexture = static_cast<D3D12Texture*>(attachment.pTexture);

			targets[i].cpuDescriptor = pTexture->GetRenderTargetView();
			targets[i].BeginningAccess.Type = Helpers::ToBeginAccess(attachment.loadOp);
			targets[i].BeginningAccess.Clear.ClearValue.Format = pTexture->GetDXGIFormat();
			targets[i].BeginningAccess.Clear.ClearValue.Color[0] = attachment.clearColor.r;
			targets[i].BeginningAccess.Clear.ClearValue.Color[1] = attachment.clearColor.g;
			targets[i].BeginningAccess.Clear.ClearValue.Color[2] = attachment.clearColor.b;
			targets[i].BeginningAccess.Clear.ClearValue.Color[3] = attachment.clearColor.a;
			targets[i].EndingAccess.Type = Helpers::ToEndAccess(attachment.storeOp);
		}

		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthTarget = {};
		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* pDepthTarget = nullptr;

		if (desc.depth.pTexture != nullptr)
		{
			auto* pTexture = static_cast<D3D12Texture*>(desc.depth.pTexture);

			depthTarget.cpuDescriptor = pTexture->GetDepthStencilView();
			depthTarget.DepthBeginningAccess.Type = Helpers::ToBeginAccess(desc.depth.loadOp);
			depthTarget.DepthBeginningAccess.Clear.ClearValue.Format = pTexture->GetDXGIFormat();
			depthTarget.DepthBeginningAccess.Clear.ClearValue.DepthStencil = { desc.depth.clearDepth, desc.depth.clearStencil };
			depthTarget.StencilBeginningAccess = depthTarget.DepthBeginningAccess;
			depthTarget.DepthEndingAccess.Type = Helpers::ToEndAccess(desc.depth.storeOp);
			depthTarget.StencilEndingAccess.Type = depthTarget.DepthEndingAccess.Type;

			pDepthTarget = &depthTarget;
		}

		m_list->BeginRenderPass(desc.colorTargetCount, targets, pDepthTarget, D3D12_RENDER_PASS_FLAG_NONE);

		D3D12_VIEWPORT viewport = { 0.0f, 0.0f, f32(desc.width), f32(desc.height), 0.0f, 1.0f };
		D3D12_RECT scissor = { 0, 0, LONG(desc.width), LONG(desc.height) };

		m_list->RSSetViewports(1, &viewport);
		m_list->RSSetScissorRects(1, &scissor);
	}

	void D3D12CommandList::EndRendering()
	{
		m_list->EndRenderPass();
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
		if (count > 16)
		{
			Terminal::Error("D3D12CommandList", "Viewport batch limit exceeded, {} > 16", count);
			return;
		}

		D3D12_VIEWPORT native[16] = {};

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
		if (count > 16)
		{
			Terminal::Error("D3D12CommandList", "Scissor batch limit exceeded, {} > 16", count);
			return;
		}

		D3D12_RECT native[16] = {};

		for (u32 i = 0; i < count; i++)
		{
			native[i].left = pScissors[i].x;
			native[i].top = pScissors[i].y;
			native[i].right = pScissors[i].x + LONG(pScissors[i].width);
			native[i].bottom = pScissors[i].y + LONG(pScissors[i].height);
		}

		m_list->RSSetScissorRects(count, native);
	}

	void D3D12CommandList::BindIndexBuffer(GfxBuffer* pBuffer, GfxIndexType type)
	{
		D3D12_INDEX_BUFFER_VIEW indexBufferView = {};

		indexBufferView.BufferLocation = pBuffer->GetGpuAddress();
		indexBufferView.SizeInBytes = u32(pBuffer->GetDesc().size);
		indexBufferView.Format = Helpers::ToIndexFormat(type);

		m_list->IASetIndexBuffer(&indexBufferView);
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
		m_list->ExecuteIndirect(m_device->GetDrawSignature(), drawCount,
			static_cast<D3D12Buffer*>(pArgs)->GetResource(), offset, nullptr, 0);
	}

	void D3D12CommandList::DrawIndexedIndirect(GfxBuffer* pArgs, usize offset, u32 drawCount)
	{
		m_list->ExecuteIndirect(m_device->GetDrawIndexedSignature(), drawCount,
			static_cast<D3D12Buffer*>(pArgs)->GetResource(), offset, nullptr, 0);
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
		m_list->ExecuteIndirect(m_device->GetDispatchSignature(), 1,
			static_cast<D3D12Buffer*>(pArgs)->GetResource(), offset, nullptr, 0);
	}

	void D3D12CommandList::CopyBuffer(GfxBuffer* pSrc, usize srcOff, GfxBuffer* pDst, usize dstOff, usize size)
	{
		m_list->CopyBufferRegion(static_cast<D3D12Buffer*>(pDst)->GetResource(), dstOff,
			static_cast<D3D12Buffer*>(pSrc)->GetResource(), srcOff, size);
	}

	void D3D12CommandList::CopyBufferToTexture(GfxBuffer* pSrc, usize srcOff, GfxTexture* pDst, u32 mipLevel, u32 arraySlice)
	{
		ID3D12Resource* pDstResource = static_cast<D3D12Texture*>(pDst)->GetResource();

		const D3D12_RESOURCE_DESC resourceDesc = pDstResource->GetDesc();
		const u32 subresource = mipLevel + arraySlice * resourceDesc.MipLevels;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};

		m_device->Handle()->GetCopyableFootprints(&resourceDesc, subresource, 1, srcOff,
			&footprint, nullptr, nullptr, nullptr);

		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};

		srcLocation.pResource = static_cast<D3D12Buffer*>(pSrc)->GetResource();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		srcLocation.PlacedFootprint = footprint;

		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};

		dstLocation.pResource = pDstResource;
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = subresource;

		m_list->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
	}

	void D3D12CommandList::CopyTextureToBuffer(GfxTexture* pSrc, u32 mipLevel, u32 arraySlice, GfxBuffer* pDst, usize dstOff)
	{
		ID3D12Resource* pSrcResource = static_cast<D3D12Texture*>(pSrc)->GetResource();

		const D3D12_RESOURCE_DESC resourceDesc = pSrcResource->GetDesc();
		const u32 subresource = mipLevel + arraySlice * resourceDesc.MipLevels;

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};

		m_device->Handle()->GetCopyableFootprints(&resourceDesc, subresource, 1, dstOff,
			&footprint, nullptr, nullptr, nullptr);

		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};

		srcLocation.pResource = pSrcResource;
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		srcLocation.SubresourceIndex = subresource;

		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};

		dstLocation.pResource = static_cast<D3D12Buffer*>(pDst)->GetResource();
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dstLocation.PlacedFootprint = footprint;

		m_list->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, nullptr);
	}
}