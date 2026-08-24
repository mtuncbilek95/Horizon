#pragma once

#include <Runtime/RHI/Command/GfxCommandList.h>
#include <Runtime/D3D12/D3D12Helpers.h>

namespace Horizon::RHI
{
	class D3D12Device;

	class D3D12CommandList final : public GfxCommandList
	{
		friend class D3D12Device;
	public:
		~D3D12CommandList() final;

		void Begin() final;
		void End() final;

		void* GetAPIHandle() const final { return m_list; }

		void BindDescriptorHeaps(GfxDescriptorHeap* pResourceHeap, GfxDescriptorHeap* pSamplerHeap) final;

		void Barrier(const GfxTextureBarrier* pBarriers, u32 count) final;
		void Barrier(const GfxBufferBarrier* pBarriers, u32 count) final;
		void BarrierUav() final;

		void BeginRendering(const GfxRenderBeginDesc& desc) final;
		void EndRendering() final;

		void BindPipeline(GfxPipeline* pPipeline) final;
		void SetGraphicsConstants(const void* pData, u32 count32, u32 offset32) final;
		void SetComputeConstants(const void* pData, u32 count32, u32 offset32) final;

		void SetViewports(const GfxViewport* pViewports, u32 count) final;
		void SetScissors(const GfxScissor* pScissors, u32 count) final;

		void BindIndexBuffer(GfxBuffer* pBuffer, GfxIndexType type) final;

		void Draw(u32 vtxCount, u32 instCount, u32 firstVtx, u32 firstInst) final;
		void DrawIndexed(u32 idxCount, u32 instCount, u32 firstIdx, i32 vtxOffset, u32 firstInst) final;
		void DrawIndirect(GfxBuffer* pArgs, usize offset, u32 drawCount) final;
		void DrawIndexedIndirect(GfxBuffer* pArgs, usize offset, u32 drawCount) final;

		void Dispatch(u32 groupX, u32 groupY, u32 groupZ) final;
		void DispatchMesh(u32 groupX, u32 groupY, u32 groupZ) final;
		void DispatchIndirect(GfxBuffer* pArgs, usize offset) final;

		void CopyBuffer(GfxBuffer* pSrc, usize srcOff, GfxBuffer* pDst, usize dstOff, usize size) final;
		void CopyBufferToTexture(GfxBuffer* pSrc, usize srcOff, GfxTexture* pDst, u32 mipLevel, u32 arraySlice) final;
		void CopyTextureToBuffer(GfxTexture* pSrc, u32 mipLevel, u32 arraySlice, GfxBuffer* pDst, usize dstOff) final;

		ID3D12GraphicsCommandList6* Handle() const { return m_list; }

	private:
		ID3D12GraphicsCommandList6* m_list = nullptr;
		ID3D12CommandAllocator* m_allocator = nullptr;
		D3D12Device* m_device = nullptr;
		D3D12_COMMAND_LIST_TYPE m_type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		b8 m_rendering = false;
	};
}
