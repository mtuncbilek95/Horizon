#pragma once

#include <Runtime/RHI/GfxTypes.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon
{
	class GfxTexture;
	class GfxBuffer;
	class GfxPipeline;

	struct GfxColorAttachment
	{
		GfxTexture* pTexture = nullptr;
		GfxLoadOp loadOp = GfxLoadOp::Clear;
		GfxStoreOp storeOp = GfxStoreOp::Store;
		GfxColor clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	};

	struct GfxDepthAttachment
	{
		GfxTexture* pTexture = nullptr;
		GfxLoadOp loadOp = GfxLoadOp::Clear;
		GfxStoreOp storeOp = GfxStoreOp::Store;
		f32 clearDepth = 1.0f;
		u8 clearStencil = 0;
	};

	struct GfxRenderBeginDesc
	{
		GfxColorAttachment colorTargets[8];
		u32 colorTargetCount = 0;
		GfxDepthAttachment depth;
		u32 width = 0, height = 0;

		GfxRenderBeginDesc& AddColorTarget(GfxTexture* pTex, GfxLoadOp op = GfxLoadOp::Clear,
			const GfxColor& clear = { 0.0f, 0.0f, 0.0f, 1.0f })
		{
			colorTargets[colorTargetCount++] = { pTex, op, GfxStoreOp::Store, clear };
			return *this;
		}

		GfxRenderBeginDesc& SetDepth(GfxTexture* pTex, GfxLoadOp op = GfxLoadOp::Clear, f32 clear = 1.0f)
		{
			depth = { pTex, op, GfxStoreOp::Store, clear, 0 };
			return *this;
		}

		GfxRenderBeginDesc& SetSize(u32 w, u32 h)
		{
			width = w;
			height = h;
			return *this;
		}
	};

	struct GfxTextureBarrier
	{
		GfxTexture* pTexture = nullptr;
		GfxResourceState before = GfxResourceState::Common;
		GfxResourceState after = GfxResourceState::Common;
	};

	struct GfxBufferBarrier
	{
		GfxBuffer* pBuffer = nullptr;
		GfxResourceState before = GfxResourceState::Common;
		GfxResourceState after = GfxResourceState::Common;
	};

	class GfxCommandList : public GfxObject
	{
	public:
		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual void* GetAPIHandle() const = 0;

		virtual void SetupBindless() = 0;

		virtual void Barrier(const GfxTextureBarrier* pBarriers, u32 count) = 0;
		virtual void Barrier(const GfxBufferBarrier* pBarriers, u32 count) = 0;
		virtual void BarrierUav() = 0;

		virtual void BeginRendering(const GfxRenderBeginDesc& desc) = 0;
		virtual void EndRendering() = 0;

		virtual void BindPipeline(GfxPipeline* pPipeline) = 0;
		virtual void SetGraphicsConstants(const void* pData, u32 count32, u32 offset32 = 0) = 0;
		virtual void SetComputeConstants(const void* pData, u32 count32, u32 offset32 = 0) = 0;

		virtual void SetViewports(const GfxViewport* pViewports, u32 count) = 0;
		virtual void SetScissors(const GfxScissor* pScissors, u32 count) = 0;

		virtual void BindIndexBuffer(GfxBuffer* pBuffer, GfxIndexType type = GfxIndexType::Index32) = 0;

		virtual void Draw(u32 vtxCount, u32 instCount, u32 firstVtx = 0, u32 firstInst = 0) = 0;
		virtual void DrawIndexed(u32 idxCount, u32 instCount, u32 firstIdx = 0, i32 vtxOffset = 0, u32 firstInst = 0) = 0;
		virtual void DrawIndirect(GfxBuffer* pArgs, usize offset, u32 drawCount) = 0;
		virtual void DrawIndexedIndirect(GfxBuffer* pArgs, usize offset, u32 drawCount) = 0;

		virtual void Dispatch(u32 groupX, u32 groupY, u32 groupZ) = 0;
		virtual void DispatchMesh(u32 groupX, u32 groupY, u32 groupZ) = 0;
		virtual void DispatchIndirect(GfxBuffer* pArgs, usize offset) = 0;

		virtual void CopyBuffer(GfxBuffer* pSrc, usize srcOff, GfxBuffer* pDst, usize dstOff, usize size) = 0;
		virtual void CopyBufferToTexture(GfxBuffer* pSrc, usize srcOff, GfxTexture* pDst, u32 mipLevel = 0, u32 arraySlice = 0) = 0;
		virtual void CopyTextureToBuffer(GfxTexture* pSrc, u32 mipLevel, u32 arraySlice, GfxBuffer* pDst, usize dstOff) = 0;

		void SetViewport(const GfxViewport& viewport) { SetViewports(&viewport, 1); }
		void SetScissor(const GfxScissor& scissor) { SetScissors(&scissor, 1); }

		GfxQueueType GetQueueType() const { return m_queueType; }

	protected:
		GfxQueueType m_queueType = GfxQueueType::Graphics;
	};
}