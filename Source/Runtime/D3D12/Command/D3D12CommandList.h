#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RHI/Command/CommandList.h>

#include <dxgi1_6.h>
#include <d3d12.h>

namespace Horizon::RHI
{
	class D3D12Device;

	class D3D12CommandList final : public CommandList
	{
		friend class D3D12Device;
	public:
		~D3D12CommandList() final;

		void SetDebugName(const c8* pName) final;

		void Start() final;
		void End() final;

		void BeginEvent(const c8* pName) final;
		void EndEvent() final;

		void Barrier(ReadOnlyList<TextureBarrier> barriers) final;
		void Barrier(ReadOnlyList<BufferBarrier> barriers) final;
		void BarrierUav() final;

		void StartRendering(const RenderingDesc& desc) final;
		void EndRendering() final;

		void BindDescriptorHeaps(DescriptorHeap* pResourceHeap, DescriptorHeap* pSamplerHeap) final;
		void BindPipeline(Pipeline* pPipeline) final;
		void BindDescriptorTable(u32 tableIndex, DescriptorTable* pTable) final;
		void SetPushConstants(const void* pData, u32 size, u32 offset = 0) final;

		void SetViewport(const Viewport& viewport) final;
		void SetScissor(const Scissor& scissor) final;

		void BindVertexBuffer(u32 binding, Buffer* pBuffer, u64 offset = 0) final;
		void BindIndexBuffer(Buffer* pBuffer, IndexType type = IndexType::Index32, u64 offset = 0) final;

		void Draw(u32 vertexCount, u32 instanceCount = 1, u32 firstVertex = 0, u32 firstInstance = 0) final;
		void DrawIndexed(u32 indexCount, u32 instanceCount = 1, u32 firstIndex = 0, i32 vertexOffset = 0, u32 firstInstance = 0) final;
		void DrawIndirect(Buffer* pArgs, u64 argsOffset, u32 maxDrawCount, Buffer* pCount = nullptr, u64 countOffset = 0) final;
		void DrawIndirectIndexed(Buffer* pArgs, u64 argsOffset, u32 maxDrawCount, Buffer* pCount = nullptr, u64 countOffset = 0) final;

		void DispatchCompute(u32 groupX, u32 groupY, u32 groupZ) final;
		void DispatchComputeIndirect(Buffer* pArgs, u64 argsOffset) final;
		void DispatchMesh(u32 groupX, u32 groupY, u32 groupZ) final;
		void DispatchMeshIndirect(Buffer* pArgs, u64 argsOffset, u32 maxDrawCount, Buffer* pCount = nullptr, u64 countOffset = 0) final;

		void CopyBuffer(Buffer* pSrc, u64 srcOffset, Buffer* pDst, u64 dstOffset, u64 size) final;
		void CopyBufferToTexture(Buffer* pSrc, u64 srcOffset, Texture* pDst, u32 mip = 0, u32 layer = 0) final;
		void CopyTextureToBuffer(Texture* pSrc, u32 mip, u32 layer, Buffer* pDst, u64 dstOffset) final;

		void ResolveTexture(Texture* pSrc, Texture* pDst, u32 mip = 0, u32 layer = 0) final;

		void RenderImGui(ImDrawData* pDrawData) final;

		ID3D12GraphicsCommandList7* Handle() const { return m_list; }

	private:
		ID3D12GraphicsCommandList7* m_list = nullptr;
		D3D12Device* m_d3dDevice = nullptr;

		Pipeline* m_boundPipeline = nullptr;
		ID3D12RootSignature* m_graphicsRootSignature = nullptr;
		ID3D12RootSignature* m_computeRootSignature = nullptr;

		List<D3D12_TEXTURE_BARRIER> m_textureBarriers;
		List<D3D12_BUFFER_BARRIER> m_bufferBarriers;
	};
}
