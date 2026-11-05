#include "D3D12CommandList.h"

#include <Runtime/D3D12/Buffer/D3D12Buffer.h>
#include <Runtime/D3D12/Command/D3D12CommandPool.h>
#include <Runtime/D3D12/Descriptor/D3D12DescriptorHeap.h>
#include <Runtime/D3D12/Descriptor/D3D12DescriptorTable.h>
#include <Runtime/D3D12/Device/D3D12Device.h>
#include <Runtime/D3D12/Helpers/D3D12Helpers.h>
#include <Runtime/D3D12/Pipeline/D3D12ComputePipeline.h>
#include <Runtime/D3D12/Pipeline/D3D12GraphicsPipeline.h>
#include <Runtime/D3D12/Pipeline/D3D12PipelineLayout.h>
#include <Runtime/D3D12/Texture/D3D12Texture.h>
#include <Runtime/D3D12/Texture/D3D12TextureView.h>
#include <Runtime/RHI/Command/IndirectArguments.h>
#include <Runtime/Containers/StringHelper.h>
#include <Runtime/Log/Terminal.h>

#include <imgui.h>
#include <backends/imgui_impl_dx12.h>

#include <cstring>

namespace Horizon::RHI
{
	static_assert(sizeof(DrawArguments) == sizeof(D3D12_DRAW_ARGUMENTS));
	static_assert(sizeof(DrawIndexedArguments) == sizeof(D3D12_DRAW_INDEXED_ARGUMENTS));
	static_assert(sizeof(DispatchArguments) == sizeof(D3D12_DISPATCH_ARGUMENTS));

	D3D12CommandList::~D3D12CommandList()
	{
		if (m_list)
			m_list->Release();
	}

	void D3D12CommandList::SetDebugName(const c8* pName)
	{
		if (!m_list || !pName)
			return;

		m_list->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::strlen(pName)), pName);
	}

	void D3D12CommandList::Start()
	{
		HRESULT hr = m_list->Reset(static_cast<D3D12CommandPool*>(m_pool)->Handle(), nullptr);
		if (FAILED(hr))
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Command list reset failed");
			return;
		}

		m_boundPipeline = nullptr;
		m_graphicsRootSignature = nullptr;
		m_computeRootSignature = nullptr;
	}

	void D3D12CommandList::End()
	{
		HRESULT hr = m_list->Close();
		if (FAILED(hr))
			Terminal::Error(StringHelper::GetTypeName(this), "Command list close failed");
	}

	void D3D12CommandList::BeginEvent(const c8* pName)
	{
		m_list->BeginEvent(1, pName, static_cast<UINT>(std::strlen(pName) + 1));
	}

	void D3D12CommandList::EndEvent()
	{
		m_list->EndEvent();
	}

	void D3D12CommandList::Barrier(ReadOnlyList<TextureBarrier> barriers)
	{
		if (barriers.IsEmpty())
			return;

		m_textureBarriers.Clear();

		for (const TextureBarrier& barrier : barriers)
		{
			D3D12Texture* pTexture = static_cast<D3D12Texture*>(barrier.pTexture);

			D3D12_TEXTURE_BARRIER d3dBarrier = {};
			d3dBarrier.SyncBefore = Helpers::ToBarrierSync(barrier.before);
			d3dBarrier.SyncAfter = Helpers::ToBarrierSync(barrier.after);
			d3dBarrier.AccessBefore = Helpers::ToBarrierAccess(barrier.before);
			d3dBarrier.AccessAfter = Helpers::ToBarrierAccess(barrier.after);
			d3dBarrier.LayoutBefore = Helpers::ToBarrierLayout(barrier.before);
			d3dBarrier.LayoutAfter = Helpers::ToBarrierLayout(barrier.after);
			d3dBarrier.pResource = pTexture->Handle();
			d3dBarrier.Flags = barrier.before == ResourceState::Undefined ? D3D12_TEXTURE_BARRIER_FLAG_DISCARD : D3D12_TEXTURE_BARRIER_FLAG_NONE;

			b8 isWholeResource = barrier.baseMip == 0 && barrier.baseLayer == 0 && barrier.mipCount == kInvalid32 && barrier.layerCount == kInvalid32;
			if (isWholeResource)
			{
				d3dBarrier.Subresources.IndexOrFirstMipLevel = 0xFFFFFFFF;
			}
			else
			{
				TextureFormat format = pTexture->GetFormat();
				b8 hasStencil = format == TextureFormat::D24_UNORM_S8_UINT || format == TextureFormat::D32_FLOAT_S8_UINT;

				d3dBarrier.Subresources.IndexOrFirstMipLevel = barrier.baseMip;
				d3dBarrier.Subresources.NumMipLevels = barrier.mipCount == kInvalid32 ? pTexture->GetMipLevels() - barrier.baseMip : barrier.mipCount;
				d3dBarrier.Subresources.FirstArraySlice = barrier.baseLayer;
				d3dBarrier.Subresources.NumArraySlices = barrier.layerCount == kInvalid32 ? pTexture->GetArrayLayers() - barrier.baseLayer : barrier.layerCount;
				d3dBarrier.Subresources.FirstPlane = 0;
				d3dBarrier.Subresources.NumPlanes = hasStencil ? 2 : 1;
			}

			m_textureBarriers.PushBack(d3dBarrier);
		}

		D3D12_BARRIER_GROUP group = {};
		group.Type = D3D12_BARRIER_TYPE_TEXTURE;
		group.NumBarriers = static_cast<UINT32>(m_textureBarriers.GetCount());
		group.pTextureBarriers = m_textureBarriers.GetData();

		m_list->Barrier(1, &group);
	}

	void D3D12CommandList::Barrier(ReadOnlyList<BufferBarrier> barriers)
	{
		if (barriers.IsEmpty())
			return;

		m_bufferBarriers.Clear();

		for (const BufferBarrier& barrier : barriers)
		{
			D3D12_BUFFER_BARRIER d3dBarrier = {};
			d3dBarrier.SyncBefore = Helpers::ToBarrierSync(barrier.before);
			d3dBarrier.SyncAfter = Helpers::ToBarrierSync(barrier.after);
			d3dBarrier.AccessBefore = Helpers::ToBarrierAccess(barrier.before);
			d3dBarrier.AccessAfter = Helpers::ToBarrierAccess(barrier.after);
			d3dBarrier.pResource = static_cast<D3D12Buffer*>(barrier.pBuffer)->Handle();
			d3dBarrier.Offset = 0;
			d3dBarrier.Size = UINT64_MAX;

			m_bufferBarriers.PushBack(d3dBarrier);
		}

		D3D12_BARRIER_GROUP group = {};
		group.Type = D3D12_BARRIER_TYPE_BUFFER;
		group.NumBarriers = static_cast<UINT32>(m_bufferBarriers.GetCount());
		group.pBufferBarriers = m_bufferBarriers.GetData();

		m_list->Barrier(1, &group);
	}

	void D3D12CommandList::BarrierUav()
	{
		D3D12_GLOBAL_BARRIER barrier = {};
		barrier.SyncBefore = D3D12_BARRIER_SYNC_ALL_SHADING;
		barrier.SyncAfter = D3D12_BARRIER_SYNC_ALL_SHADING;
		barrier.AccessBefore = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;
		barrier.AccessAfter = D3D12_BARRIER_ACCESS_UNORDERED_ACCESS;

		D3D12_BARRIER_GROUP group = {};
		group.Type = D3D12_BARRIER_TYPE_GLOBAL;
		group.NumBarriers = 1;
		group.pGlobalBarriers = &barrier;

		m_list->Barrier(1, &group);
	}

	void D3D12CommandList::StartRendering(const RenderingDesc& desc)
	{
		D3D12_RENDER_PASS_RENDER_TARGET_DESC renderTargets[kMaxColorAttachments] = {};

		for (u32 i = 0; i < desc.colorAttachmentCount; i++)
		{
			const ColorAttachment& attachment = desc.colorAttachments[i];
			D3D12TextureView* pView = static_cast<D3D12TextureView*>(attachment.pView);

			D3D12_RENDER_PASS_RENDER_TARGET_DESC& target = renderTargets[i];
			target.cpuDescriptor = pView->RenderTargetHandle();
			target.BeginningAccess.Type = Helpers::ToBeginningAccess(attachment.loadOp);
			target.BeginningAccess.Clear.ClearValue.Format = Helpers::ToFormat(pView->GetFormat());
			target.BeginningAccess.Clear.ClearValue.Color[0] = attachment.clearColor[0];
			target.BeginningAccess.Clear.ClearValue.Color[1] = attachment.clearColor[1];
			target.BeginningAccess.Clear.ClearValue.Color[2] = attachment.clearColor[2];
			target.BeginningAccess.Clear.ClearValue.Color[3] = attachment.clearColor[3];
			target.EndingAccess.Type = Helpers::ToEndingAccess(attachment.storeOp);
		}

		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthStencil = {};
		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* pDepthStencil = nullptr;
		D3D12_RENDER_PASS_FLAGS flags = D3D12_RENDER_PASS_FLAG_NONE;

		if (desc.depthAttachment.pView)
		{
			const DepthAttachment& attachment = desc.depthAttachment;
			D3D12TextureView* pView = static_cast<D3D12TextureView*>(attachment.pView);

			TextureFormat format = pView->GetFormat();
			b8 hasStencil = format == TextureFormat::D24_UNORM_S8_UINT || format == TextureFormat::D32_FLOAT_S8_UINT;

			if (attachment.readOnly)
			{
				depthStencil.cpuDescriptor = pView->DepthStencilReadOnlyHandle();

				depthStencil.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
				depthStencil.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;

				depthStencil.StencilBeginningAccess.Type = hasStencil ? D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE : D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
				depthStencil.StencilEndingAccess.Type = hasStencil ? D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE : D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;

				flags |= D3D12_RENDER_PASS_FLAG_BIND_READ_ONLY_DEPTH;
				if (hasStencil)
					flags |= D3D12_RENDER_PASS_FLAG_BIND_READ_ONLY_STENCIL;
			}
			else
			{
				depthStencil.cpuDescriptor = pView->DepthStencilHandle();

				depthStencil.DepthBeginningAccess.Type = Helpers::ToBeginningAccess(attachment.loadOp);
				depthStencil.DepthBeginningAccess.Clear.ClearValue.Format = Helpers::ToFormat(format);
				depthStencil.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = attachment.clearDepth;
				depthStencil.DepthEndingAccess.Type = Helpers::ToEndingAccess(attachment.storeOp);

				depthStencil.StencilBeginningAccess.Type = hasStencil ? Helpers::ToBeginningAccess(attachment.loadOp) : D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
				depthStencil.StencilBeginningAccess.Clear.ClearValue.Format = Helpers::ToFormat(format);
				depthStencil.StencilBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = attachment.clearStencil;
				depthStencil.StencilEndingAccess.Type = hasStencil ? Helpers::ToEndingAccess(attachment.storeOp) : D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
			}

			pDepthStencil = &depthStencil;
		}

		m_list->BeginRenderPass(desc.colorAttachmentCount, renderTargets, pDepthStencil, flags);
	}

	void D3D12CommandList::EndRendering()
	{
		m_list->EndRenderPass();
	}

	void D3D12CommandList::BindDescriptorHeaps(DescriptorHeap* pResourceHeap, DescriptorHeap* pSamplerHeap)
	{
		ID3D12DescriptorHeap* heaps[2] = {};
		u32 heapCount = 0;

		if (pResourceHeap)
			heaps[heapCount++] = static_cast<D3D12DescriptorHeap*>(pResourceHeap)->Handle();

		if (pSamplerHeap)
			heaps[heapCount++] = static_cast<D3D12DescriptorHeap*>(pSamplerHeap)->Handle();

		m_list->SetDescriptorHeaps(heapCount, heaps);
	}

	void D3D12CommandList::BindPipeline(Pipeline* pPipeline)
	{
		m_boundPipeline = pPipeline;
		ID3D12RootSignature* pRootSignature = static_cast<D3D12PipelineLayout*>(pPipeline->GetLayout())->Handle();

		if (pPipeline->GetType() == PipelineType::Graphics)
		{
			D3D12GraphicsPipeline* pGraphicsPipeline = static_cast<D3D12GraphicsPipeline*>(pPipeline);

			if (m_graphicsRootSignature != pRootSignature)
			{
				m_list->SetGraphicsRootSignature(pRootSignature);
				m_graphicsRootSignature = pRootSignature;
			}

			m_list->SetPipelineState(pGraphicsPipeline->Handle());

			if (!pGraphicsPipeline->IsMeshPipeline())
				m_list->IASetPrimitiveTopology(Helpers::ToPrimitiveTopology(pGraphicsPipeline->GetTopology()));
		}
		else
		{
			if (m_computeRootSignature != pRootSignature)
			{
				m_list->SetComputeRootSignature(pRootSignature);
				m_computeRootSignature = pRootSignature;
			}

			m_list->SetPipelineState(static_cast<D3D12ComputePipeline*>(pPipeline)->Handle());
		}
	}

	void D3D12CommandList::BindDescriptorTable(u32 tableIndex, DescriptorTable* pTable)
	{
		if (!m_boundPipeline)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "BindDescriptorTable needs a bound pipeline, table: {}", tableIndex);
			return;
		}

		u32 rootIndex = static_cast<D3D12PipelineLayout*>(m_boundPipeline->GetLayout())->TableRootIndex(tableIndex);
		D3D12_GPU_DESCRIPTOR_HANDLE handle = static_cast<D3D12DescriptorTable*>(pTable)->GpuHandle();

		if (m_boundPipeline->GetType() == PipelineType::Graphics)
			m_list->SetGraphicsRootDescriptorTable(rootIndex, handle);
		else
			m_list->SetComputeRootDescriptorTable(rootIndex, handle);
	}

	void D3D12CommandList::SetPushConstants(const void* pData, u32 size, u32 offset)
	{
		if (!m_boundPipeline || m_boundPipeline->GetLayout()->GetPushConstantSize() == 0)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "SetPushConstants needs a bound pipeline with push constants");
			return;
		}

		if (m_boundPipeline->GetType() == PipelineType::Graphics)
			m_list->SetGraphicsRoot32BitConstants(0, size / 4, pData, offset / 4);
		else
			m_list->SetComputeRoot32BitConstants(0, size / 4, pData, offset / 4);
	}

	void D3D12CommandList::SetViewport(const Viewport& viewport)
	{
		D3D12_VIEWPORT d3dViewport = {};
		d3dViewport.TopLeftX = viewport.x;
		d3dViewport.TopLeftY = viewport.y;
		d3dViewport.Width = viewport.width;
		d3dViewport.Height = viewport.height;
		d3dViewport.MinDepth = viewport.minDepth;
		d3dViewport.MaxDepth = viewport.maxDepth;

		m_list->RSSetViewports(1, &d3dViewport);
	}

	void D3D12CommandList::SetScissor(const Scissor& scissor)
	{
		D3D12_RECT rect = {};
		rect.left = scissor.x;
		rect.top = scissor.y;
		rect.right = scissor.x + scissor.width;
		rect.bottom = scissor.y + scissor.height;

		m_list->RSSetScissorRects(1, &rect);
	}

	void D3D12CommandList::BindVertexBuffer(u32 binding, Buffer* pBuffer, u64 offset)
	{
		if (!m_boundPipeline || m_boundPipeline->GetType() != PipelineType::Graphics)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "BindVertexBuffer needs a bound graphics pipeline, binding: {}", binding);
			return;
		}

		u32 stride = kInvalid32;
		for (const VertexBinding& vertexBinding : static_cast<GraphicsPipeline*>(m_boundPipeline)->GetVertexBindings())
		{
			if (vertexBinding.binding == binding)
				stride = vertexBinding.stride;
		}

		if (stride == kInvalid32)
		{
			Terminal::Error(StringHelper::GetTypeName(this), "Bound pipeline has no vertex binding {}", binding);
			return;
		}

		D3D12_VERTEX_BUFFER_VIEW view = {};
		view.BufferLocation = pBuffer->GetGpuAddress() + offset;
		view.SizeInBytes = static_cast<UINT>(pBuffer->GetSize() - offset);
		view.StrideInBytes = stride;

		m_list->IASetVertexBuffers(binding, 1, &view);
	}

	void D3D12CommandList::BindIndexBuffer(Buffer* pBuffer, IndexType type, u64 offset)
	{
		D3D12_INDEX_BUFFER_VIEW view = {};
		view.BufferLocation = pBuffer->GetGpuAddress() + offset;
		view.SizeInBytes = static_cast<UINT>(pBuffer->GetSize() - offset);
		view.Format = Helpers::ToIndexFormat(type);

		m_list->IASetIndexBuffer(&view);
	}

	void D3D12CommandList::Draw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
	{
		m_list->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
	}

	void D3D12CommandList::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset, u32 firstInstance)
	{
		m_list->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void D3D12CommandList::DrawIndirect(Buffer* pArgs, u64 argsOffset, u32 maxDrawCount, Buffer* pCount, u64 countOffset)
	{
		ID3D12Resource* pCountResource = pCount ? static_cast<D3D12Buffer*>(pCount)->Handle() : nullptr;
		m_list->ExecuteIndirect(m_d3dDevice->DrawSignature(), maxDrawCount, static_cast<D3D12Buffer*>(pArgs)->Handle(), argsOffset, pCountResource, countOffset);
	}

	void D3D12CommandList::DrawIndirectIndexed(Buffer* pArgs, u64 argsOffset, u32 maxDrawCount, Buffer* pCount, u64 countOffset)
	{
		ID3D12Resource* pCountResource = pCount ? static_cast<D3D12Buffer*>(pCount)->Handle() : nullptr;
		m_list->ExecuteIndirect(m_d3dDevice->DrawIndexedSignature(), maxDrawCount, static_cast<D3D12Buffer*>(pArgs)->Handle(), argsOffset, pCountResource, countOffset);
	}

	void D3D12CommandList::DispatchCompute(u32 groupX, u32 groupY, u32 groupZ)
	{
		m_list->Dispatch(groupX, groupY, groupZ);
	}

	void D3D12CommandList::DispatchComputeIndirect(Buffer* pArgs, u64 argsOffset)
	{
		m_list->ExecuteIndirect(m_d3dDevice->DispatchSignature(), 1, static_cast<D3D12Buffer*>(pArgs)->Handle(), argsOffset, nullptr, 0);
	}

	void D3D12CommandList::DispatchMesh(u32 groupX, u32 groupY, u32 groupZ)
	{
		m_list->DispatchMesh(groupX, groupY, groupZ);
	}

	void D3D12CommandList::DispatchMeshIndirect(Buffer* pArgs, u64 argsOffset, u32 maxDrawCount, Buffer* pCount, u64 countOffset)
	{
		ID3D12Resource* pCountResource = pCount ? static_cast<D3D12Buffer*>(pCount)->Handle() : nullptr;
		m_list->ExecuteIndirect(m_d3dDevice->DispatchMeshSignature(), maxDrawCount, static_cast<D3D12Buffer*>(pArgs)->Handle(), argsOffset, pCountResource, countOffset);
	}

	void D3D12CommandList::CopyBuffer(Buffer* pSrc, u64 srcOffset, Buffer* pDst, u64 dstOffset, u64 size)
	{
		m_list->CopyBufferRegion(static_cast<D3D12Buffer*>(pDst)->Handle(), dstOffset, static_cast<D3D12Buffer*>(pSrc)->Handle(), srcOffset, size);
	}

	void D3D12CommandList::CopyBufferToTexture(Buffer* pSrc, u64 srcOffset, Texture* pDst, u32 mip, u32 layer)
	{
		D3D12Texture* pTexture = static_cast<D3D12Texture*>(pDst);
		D3D12_RESOURCE_DESC resourceDesc = pTexture->Handle()->GetDesc();
		u32 subresource = mip + layer * pTexture->GetMipLevels();

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
		m_d3dDevice->Handle()->GetCopyableFootprints(&resourceDesc, subresource, 1, srcOffset, &footprint, nullptr, nullptr, nullptr);

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = pTexture->Handle();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dst.SubresourceIndex = subresource;

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = static_cast<D3D12Buffer*>(pSrc)->Handle();
		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		src.PlacedFootprint = footprint;

		m_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	}

	void D3D12CommandList::CopyTextureToBuffer(Texture* pSrc, u32 mip, u32 layer, Buffer* pDst, u64 dstOffset)
	{
		D3D12Texture* pTexture = static_cast<D3D12Texture*>(pSrc);
		D3D12_RESOURCE_DESC resourceDesc = pTexture->Handle()->GetDesc();
		u32 subresource = mip + layer * pTexture->GetMipLevels();

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
		m_d3dDevice->Handle()->GetCopyableFootprints(&resourceDesc, subresource, 1, dstOffset, &footprint, nullptr, nullptr, nullptr);

		D3D12_TEXTURE_COPY_LOCATION src = {};
		src.pResource = pTexture->Handle();
		src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		src.SubresourceIndex = subresource;

		D3D12_TEXTURE_COPY_LOCATION dst = {};
		dst.pResource = static_cast<D3D12Buffer*>(pDst)->Handle();
		dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		dst.PlacedFootprint = footprint;

		m_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
	}

	void D3D12CommandList::RenderImGui(ImDrawData* pDrawData)
	{
		ImGui_ImplDX12_RenderDrawData(pDrawData, m_list);

		m_boundPipeline = nullptr;
		m_graphicsRootSignature = nullptr;
	}
}
