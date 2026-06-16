#pragma once

#include <Runtime/Graphics/RHI/GfxTypes.h>
#include <Runtime/Graphics/RHI/Object/GfxResource.h>

#include <glm/glm.hpp>

namespace Horizon
{
	class GfxTexture;
	class GfxBuffer;
	class GfxPipeline;

	struct GfxColorAttachment
	{
		GfxTexture* pTexture = nullptr;
		GfxLoadOp loadOp = GfxLoadOp::Clear;
		glm::vec4 clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
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
			const glm::vec4& clear = { 0.0f, 0.0f, 0.0f, 1.0f })
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

		virtual void SetupBindless() = 0;
		virtual void Barrier(const GfxTextureBarrier* barriers, u32 count) = 0;
		virtual void BeginRendering(const GfxRenderBeginDesc& desc) = 0;
		virtual void BindPipeline(GfxPipeline* pipeline) = 0;
		virtual void SetGraphicsConstants(const void* data, u32 count32, u32 offset32 = 0) = 0;
		virtual void BindIndexBuffer(GfxBuffer* buffer) = 0;
		virtual void Draw(u32 vtxCount, u32 instCount, u32 firstVtx = 0, u32 firstInst = 0) = 0;
		virtual void DrawIndexed(u32 idxCount, u32 instCount, u32 firstIdx = 0, i32 vtxOffset = 0, u32 firstInst = 0) = 0;
		virtual void Dispatch(u32 groupX, u32 groupY, u32 groupZ) = 0;
		virtual void DispatchMesh(u32 groupX, u32 groupY, u32 groupZ) = 0;
		virtual void CopyBuffer(GfxBuffer* src, usize srcOff, GfxBuffer* dst, usize dstOff, usize size) = 0;
	};
}