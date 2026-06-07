#include <Runtime/Graphics/RHI/GfxCommandList.h>
#include <Runtime/Graphics/RHI/GfxDevice.h>
#include <Runtime/Graphics/RHI/GfxBuffer.h>
#include <Runtime/Graphics/RHI/GfxTexture.h>

#include <Runtime/Graphics/RHI/GfxPipeline.h>

#include <Runtime/Graphics/DX12/DX12Device.h>
#include <Runtime/Graphics/DX12/DX12Buffer.h>
#include <Runtime/Graphics/DX12/DX12Texture.h>
#include <Runtime/Graphics/DX12/DX12Pipeline.h>
#include <Runtime/Graphics/DX12/DX12Utils.h>

#include <vector>

namespace Horizon
{
	static D3D12_COMMAND_LIST_TYPE ToCmdListType(GfxQueueType type)
	{
		switch (type)
		{
		case GfxQueueType::Compute: return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		case GfxQueueType::Copy:    return D3D12_COMMAND_LIST_TYPE_COPY;
		default:                    return D3D12_COMMAND_LIST_TYPE_DIRECT;
		}
	}

	GfxCommandList::GfxCommandList(GfxDevice* pDevice, GfxQueueType type, GfxHandle allocator)
		: m_device(pDevice), m_type(type)
	{
		DX12Device* deviceNative = static_cast<DX12Device*>(pDevice->GetNative());
		ID3D12CommandAllocator* alloc = static_cast<ID3D12CommandAllocator*>(allocator);

		ID3D12GraphicsCommandList6* list = nullptr;
		deviceNative->device->CreateCommandList(0, ToCmdListType(type), alloc, nullptr, IID_PPV_ARGS(&list));
		list->Close();

		m_native = list;
	}

	GfxCommandList::~GfxCommandList()
	{
		ID3D12GraphicsCommandList6* list = static_cast<ID3D12GraphicsCommandList6*>(m_native);
		if (list)
			list->Release();
	}

	void GfxCommandList::Reset(GfxHandle allocator)
	{
		DX12Device* deviceNative = static_cast<DX12Device*>(m_device->GetNative());
		ID3D12GraphicsCommandList6* list = static_cast<ID3D12GraphicsCommandList6*>(m_native);
		ID3D12CommandAllocator* alloc = static_cast<ID3D12CommandAllocator*>(allocator);

		list->Reset(alloc, nullptr);

		if (m_type == GfxQueueType::Copy)
			return;

		ID3D12DescriptorHeap* heaps[] = { deviceNative->bindless.heap };
		list->SetDescriptorHeaps(1, heaps);

		if (m_type == GfxQueueType::Graphics)
			list->SetGraphicsRootSignature(deviceNative->globalRootSignature);
		list->SetComputeRootSignature(deviceNative->globalRootSignature);
	}

	void GfxCommandList::Close()
	{
		static_cast<ID3D12GraphicsCommandList6*>(m_native)->Close();
	}

	void GfxCommandList::Barrier(std::span<const GfxTextureBarrier> barriers)
	{
		ID3D12GraphicsCommandList6* list = static_cast<ID3D12GraphicsCommandList6*>(m_native);

		std::vector<D3D12_RESOURCE_BARRIER> dx(barriers.size());
		for (usize i = 0; i < barriers.size(); ++i)
		{
			DX12Texture* tex = static_cast<DX12Texture*>(barriers[i].texture->GetNative());
			dx[i] = {};
			dx[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			dx[i].Transition.pResource = tex->resource;
			dx[i].Transition.StateBefore = ToResourceState(barriers[i].before);
			dx[i].Transition.StateAfter = ToResourceState(barriers[i].after);
			dx[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		}
		list->ResourceBarrier((u32)dx.size(), dx.data());
	}

	void GfxCommandList::Barrier(std::span<const GfxBufferBarrier> barriers)
	{
		ID3D12GraphicsCommandList6* list = static_cast<ID3D12GraphicsCommandList6*>(m_native);

		std::vector<D3D12_RESOURCE_BARRIER> dx(barriers.size());
		for (usize i = 0; i < barriers.size(); ++i)
		{
			DX12Buffer* buf = static_cast<DX12Buffer*>(barriers[i].buffer->GetNative());
			dx[i] = {};
			dx[i].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			dx[i].Transition.pResource = buf->resource;
			dx[i].Transition.StateBefore = ToResourceState(barriers[i].before);
			dx[i].Transition.StateAfter = ToResourceState(barriers[i].after);
			dx[i].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		}
		list->ResourceBarrier((u32)dx.size(), dx.data());
	}

	void GfxCommandList::BeginRenderPass(const GfxRenderPassBegin& pass)
	{
		DX12Device* deviceNative = static_cast<DX12Device*>(m_device->GetNative());
		ID3D12GraphicsCommandList6* list = static_cast<ID3D12GraphicsCommandList6*>(m_native);

		D3D12_CPU_DESCRIPTOR_HANDLE rtvs[8] = {};
		for (u32 i = 0; i < pass.colorCount; ++i)
			rtvs[i] = deviceNative->rtv.GetCPUHandle(pass.colors[i].target->GetTargetView());

		D3D12_CPU_DESCRIPTOR_HANDLE dsv = {};
		D3D12_CPU_DESCRIPTOR_HANDLE* pDsv = nullptr;
		if (pass.hasDepth)
		{
			dsv = deviceNative->dsv.GetCPUHandle(pass.depth.target->GetDepthView());
			pDsv = &dsv;
		}

		list->OMSetRenderTargets(pass.colorCount, rtvs, FALSE, pDsv);

		for (u32 i = 0; i < pass.colorCount; ++i)
			if (pass.colors[i].load == GfxLoadOp::Clear)
				list->ClearRenderTargetView(rtvs[i], &pass.colors[i].clear.x, 0, nullptr);

		if (pass.hasDepth && pass.depth.load == GfxLoadOp::Clear)
			list->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, pass.depth.clearDepth, 0, 0, nullptr);

		D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (f32)pass.width, (f32)pass.height, 0.0f, 1.0f };
		D3D12_RECT scissor = { 0, 0, (LONG)pass.width, (LONG)pass.height };
		list->RSSetViewports(1, &viewport);
		list->RSSetScissorRects(1, &scissor);
	}

	void GfxCommandList::EndRenderPass()
	{
	}

	void GfxCommandList::BindPipeline(GfxPipeline* pipeline)
	{
		ID3D12GraphicsCommandList6* list = static_cast<ID3D12GraphicsCommandList6*>(m_native);
		DX12Pipeline* p = static_cast<DX12Pipeline*>(pipeline->GetNative());

		list->SetPipelineState(p->pso);
		if (!p->isCompute)
			list->IASetPrimitiveTopology(p->topology);
	}

	void GfxCommandList::SetConstants(const void* data, u32 count)
	{
		ID3D12GraphicsCommandList6* list = static_cast<ID3D12GraphicsCommandList6*>(m_native);
		list->SetGraphicsRoot32BitConstants(0, count, data, 0);
		list->SetComputeRoot32BitConstants(0, count, data, 0);
	}

	void GfxCommandList::BindIndexBuffer(GfxBuffer* buffer, GfxIndexType type)
	{
		ID3D12GraphicsCommandList6* list = static_cast<ID3D12GraphicsCommandList6*>(m_native);
		DX12Buffer* buf = static_cast<DX12Buffer*>(buffer->GetNative());

		D3D12_INDEX_BUFFER_VIEW view = {};
		view.BufferLocation = buf->resource->GetGPUVirtualAddress();
		view.SizeInBytes = (u32)buffer->GetSize();
		view.Format = (type == GfxIndexType::UInt16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		list->IASetIndexBuffer(&view);
	}

	void GfxCommandList::Draw(u32 vertexCount, u32 instanceCount)
	{
		static_cast<ID3D12GraphicsCommandList6*>(m_native)->DrawInstanced(vertexCount, instanceCount, 0, 0);
	}

	void GfxCommandList::DrawIndexed(u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset)
	{
		static_cast<ID3D12GraphicsCommandList6*>(m_native)->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, 0);
	}

	void GfxCommandList::Dispatch(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
	{
		static_cast<ID3D12GraphicsCommandList6*>(m_native)->Dispatch(groupCountX, groupCountY, groupCountZ);
	}

	void GfxCommandList::DispatchMesh(u32 groupCountX, u32 groupCountY, u32 groupCountZ)
	{
		static_cast<ID3D12GraphicsCommandList6*>(m_native)->DispatchMesh(groupCountX, groupCountY, groupCountZ);
	}
}
