#include "D3D12CommandList.h"

#include <Runtime/D3D12/D3D12Buffer.h>
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
		ID3D12DescriptorHeap* heaps[] = { m_device->GetResourceHeap() };
		m_list->SetDescriptorHeaps(1, heaps);

		ID3D12RootSignature* rootSig = m_device->GetRootSignature();
		if (m_type == D3D12_COMMAND_LIST_TYPE_DIRECT)
		{
			m_list->SetGraphicsRootSignature(rootSig);
			m_list->SetComputeRootSignature(rootSig);
		}
		else
		{
			m_list->SetComputeRootSignature(rootSig);
		}
	}

	void D3D12CommandList::Barrier(const GfxTextureBarrier* barriers, u32 count)
	{
		assert(count <= 16 && "Barrier batch limit exceeded");

		D3D12_RESOURCE_BARRIER native[16] = {};
		for (u32 i = 0; i < count; i++)
		{
			native[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			native[i].Transition.pResource = static_cast<D3D12Texture*>(barriers[i].pTexture)->GetResource();
			native[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			native[i].Transition.StateBefore = Helpers::ToResourceState(barriers[i].before);
			native[i].Transition.StateAfter = Helpers::ToResourceState(barriers[i].after);
		}

		m_list->ResourceBarrier(count, native);
	}

	void D3D12CommandList::BeginRendering(const GfxRenderBeginDesc& desc)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[8] = {};
		for (u32 i = 0; i < desc.colorTargetCount; i++)
		{
			const GfxColorAttachment& attachment = desc.colorTargets[i];
			rtvHandles[i] = static_cast<D3D12Texture*>(attachment.pTexture)->GetRenderTargetView();

			if (attachment.loadOp == GfxLoadOp::Clear)
			{
				const f32 color[4] = { attachment.clearColor.r, attachment.clearColor.g,
									   attachment.clearColor.b, attachment.clearColor.a };
				m_list->ClearRenderTargetView(rtvHandles[i], color, 0, nullptr);
			}
		}

		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};
		D3D12_CPU_DESCRIPTOR_HANDLE* pDsvHandle = nullptr;
		if (desc.depth.pTexture != nullptr)
		{
			dsvHandle = static_cast<D3D12Texture*>(desc.depth.pTexture)->GetDepthStencilView();
			pDsvHandle = &dsvHandle;

			if (desc.depth.loadOp == GfxLoadOp::Clear)
			{
				m_list->ClearDepthStencilView(dsvHandle,
					D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
					desc.depth.clearDepth, desc.depth.clearStencil, 0, nullptr);
			}
		}

		m_list->OMSetRenderTargets(desc.colorTargetCount, rtvHandles, FALSE, pDsvHandle);

		D3D12_VIEWPORT viewport = { 0.0f, 0.0f, f32(desc.width), f32(desc.height), 0.0f, 1.0f };
		D3D12_RECT scissor = { 0, 0, LONG(desc.width), LONG(desc.height) };
		m_list->RSSetViewports(1, &viewport);
		m_list->RSSetScissorRects(1, &scissor);
	}

	void D3D12CommandList::BindPipeline(GfxPipeline* pipeline)
	{
		auto* d3d12Pipeline = static_cast<D3D12Pipeline*>(pipeline);
		m_list->SetPipelineState(d3d12Pipeline->GetPipeline());

		if (d3d12Pipeline->GetType() == GfxPipelineType::Graphics && !d3d12Pipeline->UsesMeshShading())
			m_list->IASetPrimitiveTopology(d3d12Pipeline->GetTopology());
	}

	void D3D12CommandList::SetGraphicsConstants(const void* data, u32 count32, u32 offset32)
	{
		m_list->SetGraphicsRoot32BitConstants(0, count32, data, offset32);
	}

	void D3D12CommandList::SetComputeConstants(const void* data, u32 count32, u32 offset32)
	{
		m_list->SetComputeRoot32BitConstants(0, count32, data, offset32);
	}

	void D3D12CommandList::BindIndexBuffer(GfxBuffer* buffer)
	{
		D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
		indexBufferView.BufferLocation = buffer->GetGpuAddress();
		indexBufferView.SizeInBytes = u32(buffer->GetDesc().size);
		indexBufferView.Format = buffer->GetDesc().stride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		m_list->IASetIndexBuffer(&indexBufferView);
	}

	void D3D12CommandList::Draw(u32 vtx, u32 inst, u32 firstVtx, u32 firstInst)
	{
		m_list->DrawInstanced(vtx, inst, firstVtx, firstInst);
	}

	void D3D12CommandList::DrawIndexed(u32 idx, u32 inst, u32 firstIdx, i32 vtxOff, u32 firstInst)
	{
		m_list->DrawIndexedInstanced(idx, inst, firstIdx, vtxOff, firstInst);
	}

	void D3D12CommandList::Dispatch(u32 groupX, u32 groupY, u32 groupZ)
	{
		m_list->Dispatch(groupX, groupY, groupZ);
	}

	void D3D12CommandList::DispatchMesh(u32 groupX, u32 groupY, u32 groupZ)
	{
		m_list->DispatchMesh(groupX, groupY, groupZ);
	}

	void D3D12CommandList::CopyBuffer(GfxBuffer* src, usize srcOffset, GfxBuffer* dst, usize dstOffset, usize size)
	{
		m_list->CopyBufferRegion(static_cast<D3D12Buffer*>(dst)->GetResource(), dstOffset,
			static_cast<D3D12Buffer*>(src)->GetResource(), srcOffset, size);
	}

	void D3D12CommandList::CopyBufferToTexture(GfxBuffer* src, usize srcOffset, GfxTexture* dst, u32 mipLevel, u32 arraySlice)
	{
		ID3D12Resource* destinationResource = static_cast<D3D12Texture*>(dst)->GetResource();

		const D3D12_RESOURCE_DESC resourceDesc = destinationResource->GetDesc();
		const u32 subresource = mipLevel + arraySlice * resourceDesc.MipLevels;

		ID3D12Device* nativeDevice = nullptr;
		destinationResource->GetDevice(IID_PPV_ARGS(&nativeDevice));

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};
		nativeDevice->GetCopyableFootprints(&resourceDesc, subresource, 1, srcOffset,
			&footprint, nullptr, nullptr, nullptr);
		nativeDevice->Release();

		D3D12_TEXTURE_COPY_LOCATION sourceLocation = {};
		sourceLocation.pResource = static_cast<D3D12Buffer*>(src)->GetResource();
		sourceLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		sourceLocation.PlacedFootprint = footprint;

		D3D12_TEXTURE_COPY_LOCATION destinationLocation = {};
		destinationLocation.pResource = destinationResource;
		destinationLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		destinationLocation.SubresourceIndex = subresource;

		m_list->CopyTextureRegion(&destinationLocation, 0, 0, 0, &sourceLocation, nullptr);
	}
}