#pragma once

#include <Runtime/RHI/GfxTypes.h>
#include <Runtime/RHI/Object/GfxResource.h>

namespace Horizon
{
	class GfxTexture;
	class GfxBuffer;
	class GfxPipeline;

	struct GfxColorAttachment
	{
		GfxTexture* pTexture = nullptr;
		GfxLoadOp loadOp = GfxLoadOp::Clear;
		GfxColor clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
	};

	struct GfxDepthAttachment
	{
		GfxTexture* pTexture = nullptr;
		GfxLoadOp loadOp = GfxLoadOp::Clear;
		f32 clearDepth = 1.0f;
		u8 clearStencil = 0;
	};

	struct GfxRenderBeginDesc
	{
		GfxColorAttachment colorTargets[8];
		u32 colorTargetCount = 0;
		GfxDepthAttachment depth;
		u32 width = 0, height = 0;

		GfxRenderBeginDesc& addColorTarget(GfxTexture* pTex, GfxLoadOp op = GfxLoadOp::Clear,
			const GfxColor& clear = { 0.0f, 0.0f, 0.0f, 1.0f })
		{
			colorTargets[colorTargetCount++] = { pTex, op, clear };
			return *this;
		}
		GfxRenderBeginDesc& setDepth(GfxTexture* pTex, GfxLoadOp op = GfxLoadOp::Clear, f32 clear = 1.0f)
		{
			depth = { pTex, op, clear, 0 };
			return *this;
		}
		GfxRenderBeginDesc& setSize(u32 w, u32 h) { width = w; height = h; return *this; }
	};

	struct GfxTextureBarrier
	{
		GfxTexture* pTexture = nullptr;
		GfxResourceState before = GfxResourceState::Common;
		GfxResourceState after = GfxResourceState::Common;
	};

	class GfxCommandList
	{
	public:
		virtual ~GfxCommandList() = default;

		virtual void Begin() = 0;
		virtual void End() = 0;

		virtual void* GetAPIHandle() const = 0;

		virtual void SetupBindless() = 0;
		virtual void Barrier(const GfxTextureBarrier* barriers, u32 count) = 0;
		virtual void BeginRendering(const GfxRenderBeginDesc& desc) = 0;
		virtual void BindPipeline(GfxPipeline* pipeline) = 0;
		virtual void SetGraphicsConstants(const void* data, u32 count32, u32 offset32 = 0) = 0;
		virtual void SetComputeConstants(const void* data, u32 count32, u32 offset32 = 0) = 0;
		virtual void SetViewports(const GfxViewport* viewports, u32 count) = 0;
		virtual void SetScissors(const GfxScissor* scissors, u32 count) = 0;
		virtual void SetViewport(const GfxViewport& viewport) = 0;
		virtual void SetScissor(const GfxScissor& scissor) = 0;
		virtual void BindIndexBuffer(GfxBuffer* buffer) = 0;
		virtual void Draw(u32 vtxCount, u32 instCount, u32 firstVtx = 0, u32 firstInst = 0) = 0;
		virtual void DrawIndexed(u32 idxCount, u32 instCount, u32 firstIdx = 0, i32 vtxOffset = 0, u32 firstInst = 0) = 0;
		virtual void Dispatch(u32 groupX, u32 groupY, u32 groupZ) = 0;
		virtual void DispatchMesh(u32 groupX, u32 groupY, u32 groupZ) = 0;
		virtual void CopyBuffer(GfxBuffer* src, usize srcOff, GfxBuffer* dst, usize dstOff, usize size) = 0;
		virtual void CopyBufferToTexture(GfxBuffer* src, usize srcOff, GfxTexture* dst, u32 mipLevel = 0, u32 arraySlice = 0) = 0;
	};
}