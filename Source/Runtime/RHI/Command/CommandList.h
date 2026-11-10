#pragma once

#include <Runtime/Containers/Vector/ReadOnlyList.h>
#include <Runtime/RHI/Buffer/IndexType.h>
#include <Runtime/RHI/Command/BufferBarrier.h>
#include <Runtime/RHI/Command/CommandListDesc.h>
#include <Runtime/RHI/Command/RenderingDesc.h>
#include <Runtime/RHI/Command/TextureBarrier.h>
#include <Runtime/RHI/Common/Scissor.h>
#include <Runtime/RHI/Common/Viewport.h>
#include <Runtime/RHI/Object/Object.h>
#include <Runtime/RHI/Queue/QueueType.h>

struct ImDrawData;

namespace Horizon::RHI
{
	class Buffer;
	class Texture;
	class Pipeline;
	class DescriptorHeap;
	class DescriptorTable;

	class CommandList : public Object
	{
	public:
		virtual void Start() = 0;
		virtual void End() = 0;

		virtual void BeginEvent(const c8* pName) = 0;
		virtual void EndEvent() = 0;

		virtual void Barrier(ReadOnlyList<TextureBarrier> barriers) = 0;
		virtual void Barrier(ReadOnlyList<BufferBarrier> barriers) = 0;
		virtual void BarrierUav() = 0;

		virtual void StartRendering(const RenderingDesc& desc) = 0;
		virtual void EndRendering() = 0;

		virtual void BindDescriptorHeaps(DescriptorHeap* pResourceHeap, DescriptorHeap* pSamplerHeap) = 0;
		virtual void BindPipeline(Pipeline* pPipeline) = 0;
		virtual void BindDescriptorTable(u32 tableIndex, DescriptorTable* pTable) = 0;
		virtual void SetPushConstants(const void* pData, u32 size, u32 offset = 0) = 0;

		virtual void SetViewport(const Viewport& viewport) = 0;
		virtual void SetScissor(const Scissor& scissor) = 0;

		virtual void BindVertexBuffer(u32 binding, Buffer* pBuffer, u64 offset = 0) = 0;
		virtual void BindIndexBuffer(Buffer* pBuffer, IndexType type = IndexType::Index32, u64 offset = 0) = 0;

		virtual void Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0) = 0;
		virtual void DrawIndexed(u32 indexCount, u32 instanceCount = 1, u32 firstIndex = 0, i32 vertexOffset = 0, u32 firstInstance = 0) = 0;
		virtual void DrawIndirect(Buffer* pArgs, u64 argsOffset, u32 maxDrawCount, Buffer* pCount = nullptr, u64 countOffset = 0) = 0;
		virtual void DrawIndirectIndexed(Buffer* pArgs, u64 argsOffset, u32 maxDrawCount, Buffer* pCount = nullptr, u64 countOffset = 0) = 0;

		virtual void DispatchCompute(u32 groupX, u32 groupY, u32 groupZ) = 0;
		virtual void DispatchComputeIndirect(Buffer* pArgs, u64 argsOffset) = 0;
		virtual void DispatchMesh(u32 groupX, u32 groupY, u32 groupZ) = 0;
		virtual void DispatchMeshIndirect(Buffer* pArgs, u64 argsOffset, u32 maxDrawCount, Buffer* pCount = nullptr, u64 countOffset = 0) = 0;

		virtual void CopyBuffer(Buffer* pSrc, u64 srcOffset, Buffer* pDst, u64 dstOffset, u64 size) = 0;
		virtual void CopyBufferToTexture(Buffer* pSrc, u64 srcOffset, Texture* pDst, u32 mip = 0, u32 layer = 0) = 0;
		virtual void CopyTextureToBuffer(Texture* pSrc, u32 mip, u32 layer, Buffer* pDst, u64 dstOffset) = 0;

		virtual void ResolveTexture(Texture* pSrc, Texture* pDst, u32 mip = 0, u32 layer = 0) = 0;

		virtual void RenderImGui(ImDrawData* pDrawData) = 0;

		CommandPool* GetPool() const { return m_pool; }
		QueueType GetType() const { return m_type; }

	protected:
		CommandPool* m_pool = nullptr;
		QueueType m_type = QueueType::Graphics;
	};
}
