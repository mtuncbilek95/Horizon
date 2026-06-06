#include "DX12Backend.h"

#include <Engine/Graphics/RHI/GfxCommandList.h>
#include <Engine/Graphics/RHI/GfxPipeline.h>

#include <pix3.h>

namespace Horizon
{
	using namespace DX12;

	GfxCommandList* GfxDevice::AcquireCmd(GfxQueueType type, u32 workerIndex)
	{
		Context& context = GfxContext();
		WorkerCmd& worker = context.frames[context.frameIndex].workers[workerIndex];

		if (worker.nextList >= worker.listCount)
		{
			GfxCommandList& newList = worker.lists[worker.listCount];
			newList.allocator = worker.allocator;
			newList.type = type;
			context.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, worker.allocator, nullptr,
				IID_PPV_ARGS(&newList.list));

			SetDebugName(newList.list, "GfxCmdList");

			newList.list->Close();
			worker.listCount++;
		}

		GfxCommandList& cmdList = worker.lists[worker.nextList++];
		cmdList.computeMode = false;
		cmdList.list->Reset(worker.allocator, nullptr);

		cmdList.list->SetDescriptorHeaps(1, &context.bindlessHeap);

		return &cmdList;
	}

	static D3D12_RESOURCE_STATES ToState(GfxResourceState state)
	{
		switch (state)
		{
		case GfxResourceState::VertexBuffer:
		case GfxResourceState::ConstantBuffer:
			return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case GfxResourceState::IndexBuffer:
			return D3D12_RESOURCE_STATE_INDEX_BUFFER;
		case GfxResourceState::IndirectArg:
			return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
		case GfxResourceState::ShaderResource:
			return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		case GfxResourceState::UnorderedAccess:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case GfxResourceState::RenderTarget:
			return D3D12_RESOURCE_STATE_RENDER_TARGET;
		case GfxResourceState::DepthWrite:
			return D3D12_RESOURCE_STATE_DEPTH_WRITE;
		case GfxResourceState::DepthRead:
			return D3D12_RESOURCE_STATE_DEPTH_READ;
		case GfxResourceState::CopySrc:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case GfxResourceState::CopyDst:
			return D3D12_RESOURCE_STATE_COPY_DEST;
		case GfxResourceState::Present:
			return D3D12_RESOURCE_STATE_PRESENT;
		default:
			return D3D12_RESOURCE_STATE_COMMON;
		}
	}

	void GfxCmdList::Barrier(GfxCommandList* cmdList, std::span<const GfxBarrier> barriers)
	{
		D3D12_RESOURCE_BARRIER barrierList[16];
		u32 barrierCount = 0;

		for (const GfxBarrier& barrier : barriers)
		{
			DX12Texture& texture = TexturePoolGet(barrier.texture);
			barrierList[barrierCount] = {};
			barrierList[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrierList[barrierCount].Transition.pResource = texture.resource;
			barrierList[barrierCount].Transition.StateBefore = ToState(barrier.before);
			barrierList[barrierCount].Transition.StateAfter = ToState(barrier.after);
			barrierList[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrierCount++;
		}

		if (barrierCount)
			cmdList->list->ResourceBarrier(barrierCount, barrierList);
	}

	void GfxCmdList::Barrier(GfxCommandList* cmdList, std::span<const GfxBufferBarrier> barriers)
	{
		D3D12_RESOURCE_BARRIER barrierList[16];
		u32 barrierCount = 0;

		for (const GfxBufferBarrier& barrier : barriers)
		{
			ID3D12Resource* resource = BufferPoolGet(barrier.buffer).resource;
			barrierList[barrierCount] = {};

			if (barrier.before == barrier.after)
			{
				barrierList[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				barrierList[barrierCount].UAV.pResource = resource;
			}
			else
			{
				barrierList[barrierCount].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrierList[barrierCount].Transition.pResource = resource;
				barrierList[barrierCount].Transition.StateBefore = ToState(barrier.before);
				barrierList[barrierCount].Transition.StateAfter = ToState(barrier.after);
				barrierList[barrierCount].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			}
			barrierCount++;
		}

		if (barrierCount)
			cmdList->list->ResourceBarrier(barrierCount, barrierList);
	}

	void GfxCmdList::BeginMarker(GfxCommandList* cmdList, std::string_view name, const glm::uvec3& color)
	{
		PIXBeginEvent(cmdList->list, PIX_COLOR(color.x, color.y, color.z), name.data());
	}

	void GfxCmdList::EndMarker(GfxCommandList* cmdList)
	{
		PIXEndEvent(cmdList->list);
	}

	void GfxCmdList::Marker(GfxCommandList* cmdList, std::string_view name)
	{
		PIXSetMarker(cmdList->list, 0xFFFFFFFF, name.data());
	}

	void GfxCmdList::BeginRenderPass(GfxCommandList* cmdList, const GfxRenderPassBegin& renderPass)
	{
		Context& context = GfxContext();

		D3D12_CPU_DESCRIPTOR_HANDLE targetViews[8];
		for (u32 colorIdx = 0; colorIdx < renderPass.colorCount; colorIdx++)
		{
			DX12Texture& texture = TexturePoolGet(renderPass.colors[colorIdx].target);
			targetViews[colorIdx] = context.targetViewHeap->GetCPUDescriptorHandleForHeapStart();
			targetViews[colorIdx].ptr += usize(texture.targetViewIndex) * context.targetViewStride;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE depthView = {};
		D3D12_CPU_DESCRIPTOR_HANDLE* pDepthView = nullptr;
		if (renderPass.hasDepth)
		{
			DX12Texture& texture = TexturePoolGet(renderPass.depth.target);
			depthView = context.depthViewHeap->GetCPUDescriptorHandleForHeapStart();
			depthView.ptr += usize(texture.depthViewIndex) * context.depthViewStride;
			pDepthView = &depthView;
		}

		cmdList->list->OMSetRenderTargets(renderPass.colorCount, targetViews, FALSE, pDepthView);

		for (u32 colorIdx = 0; colorIdx < renderPass.colorCount; colorIdx++)
		{
			if (renderPass.colors[colorIdx].load == GfxLoadOp::Clear)
			{
				std::array<f32, 4> color = 
				{ 
					renderPass.colors[colorIdx].clear.r, 
					renderPass.colors[colorIdx].clear.g, 
					renderPass.colors[colorIdx].clear.b, 
					renderPass.colors[colorIdx].clear.a 
				};
				 cmdList->list->ClearRenderTargetView(targetViews[colorIdx], color.data(), 0, nullptr);
			}
		}

		if (renderPass.hasDepth && renderPass.depth.load == GfxLoadOp::Clear)
			cmdList->list->ClearDepthStencilView(depthView, D3D12_CLEAR_FLAG_DEPTH, renderPass.depth.clearDepth, 0, 0, nullptr);

		D3D12_VIEWPORT viewport =
		{
			0,
			0,
			(f32)renderPass.width,
			(f32)renderPass.height,
			0,
			1
		};
		D3D12_RECT scissor =
		{
			0,
			0,
			(int64_t)renderPass.width,
			(int64_t)renderPass.height
		};
		cmdList->list->RSSetViewports(1, &viewport);
		cmdList->list->RSSetScissorRects(1, &scissor);
	}

	void GfxCmdList::EndRenderPass(GfxCommandList* cmdList)
	{}

	void GfxCmdList::BindPipeline(GfxCommandList* cmdList, GfxPipelineHandle pipeline)
	{
		Context& context = GfxContext();

		DX12Pipeline& pipelineObj = PipelinePoolGet(pipeline);
		cmdList->list->SetPipelineState(pipelineObj.pso);
		cmdList->computeMode = pipelineObj.isCompute;

		if (pipelineObj.isCompute)
			cmdList->list->SetComputeRootSignature(context.rootSignature);
		else
			cmdList->list->SetGraphicsRootSignature(context.rootSignature);

		if (!pipelineObj.isCompute)
			cmdList->list->IASetPrimitiveTopology(pipelineObj.topology);
	}

	void GfxCmdList::SetConstants(GfxCommandList* cmdList, const void* data, u32 size)
	{
		u32 dwords = (size + 3) / 4;

		if (cmdList->computeMode)
			cmdList->list->SetComputeRoot32BitConstants(0, dwords, data, 0);
		else
			cmdList->list->SetGraphicsRoot32BitConstants(0, dwords, data, 0);
	}

	void GfxCmdList::BindIndexBuffer(GfxCommandList* cmdList, GfxBufferHandle buffer, GfxIndexType type)
	{
		DX12Buffer& dxBuffer = BufferPoolGet(buffer);
		D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
		indexBufferView.BufferLocation = dxBuffer.gpuVA;
		indexBufferView.SizeInBytes = (u32)dxBuffer.size;
		indexBufferView.Format = (type == GfxIndexType::UInt16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
		cmdList->list->IASetIndexBuffer(&indexBufferView);
	}

	void GfxCmdList::Draw(GfxCommandList* cmdList, u32 vertexCount, u32 instanceCount)
	{
		cmdList->list->DrawInstanced(vertexCount, instanceCount, 0, 0);
	}

	void GfxCmdList::DrawIndexed(GfxCommandList* cmdList, u32 indexCount, u32 instanceCount, u32 firstIndex, i32 vertexOffset)
	{
		cmdList->list->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, 0);
	}

	void GfxCmdList::ExecuteIndirect(GfxCommandList* cmdList, GfxCommandSignatureHandle signature, u32 maxCommandCount,
		GfxBufferHandle argBuffer, u64 argOffset, GfxBufferHandle countBuffer, u64 countOffset)
	{
		Context& context = GfxContext();
		ID3D12CommandSignature* commandSig = context.cmdSigs[signature.index()];
		ID3D12Resource* argResource = BufferPoolGet(argBuffer).resource;
		ID3D12Resource* countResource = countBuffer.isValid() ? BufferPoolGet(countBuffer).resource : nullptr;

		cmdList->list->ExecuteIndirect(commandSig, maxCommandCount, argResource, argOffset, countResource, countOffset);
	}

	void GfxCmdList::Dispatch(GfxCommandList* cmdList, u32 groupCountX, u32 groupCountY, u32 groupCountZ)
	{
		cmdList->list->Dispatch(groupCountX, groupCountY, groupCountZ);
	}

	void GfxCmdList::DispatchMesh(GfxCommandList* cmdList, u32 groupCountX, u32 groupCountY, u32 groupCountZ)
	{
		cmdList->list->DispatchMesh(groupCountX, groupCountY, groupCountZ);
	}
}
