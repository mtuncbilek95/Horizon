#pragma once

#include <Runtime/Graphics/RHI/Command/GfxCommandList.h>
#include <Runtime/Graphics/D3D12/D3D12Backend.h>

namespace Horizon
{
	class D3D12Device;

	class D3D12CommandList final : public GfxCommandList
	{
		friend class D3D12Device;
	public:
		~D3D12CommandList() final;

		void Begin() final;
		void End() final;
		void SetupBindless() final;
		void Barrier(const GfxTextureBarrier* barriers, u32 count) final;
		void BeginRendering(const GfxRenderBeginDesc& desc) final;
		void BindPipeline(GfxPipeline* pipeline) final;
		void SetGraphicsConstants(const void* data, u32 count32, u32 offset32) final;
		void SetComputeConstants(const void* data, u32 count32, u32 offset32) final;
		void BindIndexBuffer(GfxBuffer* buffer) final;
		void Draw(u32 vtx, u32 inst, u32 firstVtx, u32 firstInst) final;
		void DrawIndexed(u32 idx, u32 inst, u32 firstIdx, i32 vtxOff, u32 firstInst) final;
		void Dispatch(u32 groupX, u32 groupY, u32 groupZ) final;
		void DispatchMesh(u32 groupX, u32 groupY, u32 groupZ) final;
		void CopyBuffer(GfxBuffer* src, usize srcOff, GfxBuffer* dst, usize dstOff, usize size) final;
		void CopyBufferToTexture(GfxBuffer* src, usize srcOff, GfxTexture* dst, u32 mipLevel, u32 arraySlice) final;
		void DrawImGui() final;

		ID3D12GraphicsCommandList6* Handle() const { return m_list; }

	private:
		ID3D12GraphicsCommandList6* m_list = nullptr;
		ID3D12CommandAllocator* m_allocator = nullptr;
		D3D12Device* m_device = nullptr;
		D3D12_COMMAND_LIST_TYPE m_type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	};
}