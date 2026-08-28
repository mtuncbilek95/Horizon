#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Command/GfxColorAttachment.h>
#include <Runtime/RHI/Command/GfxDepthAttachment.h>

namespace Horizon::RHI
{
	struct GfxRenderBeginDesc
	{
		GfxColorAttachment colorTargets[8];
		u32 colorTargetCount = 0;
		GfxDepthAttachment depth;
		u32 width = 0;
		u32 height = 0;
		GfxRenderBeginDesc& AddColorTarget(GfxTexture* pTexture, GfxLoadOp op = GfxLoadOp::Clear,
			const GfxColor& clear = {})
		{
			colorTargets[colorTargetCount++] = { pTexture, op, GfxStoreOp::Store, clear };
			return *this;
		}
		GfxRenderBeginDesc& SetDepth(GfxTexture* pTexture, GfxLoadOp op = GfxLoadOp::Clear, f32 clear = 1.0f)
		{
			depth = { pTexture, op, GfxStoreOp::Store, clear, 0 };
			return *this;
		}

		GfxRenderBeginDesc& SetSize(u32 width, u32 height)
		{
			this->width = width;
			this->height = height;
			return *this;
		}
	};
}
