#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Command/GfxColorAttachment.h>
#include <Runtime/RHI/Command/GfxDepthAttachment.h>

namespace Horizon::RHI
{
	/**
	 * @brief Full description of a dynamic render pass,
	 * holding up to eight color targets, one depth target
	 * and the render area. The setters chain so a pass can
	 * be declared in a single expression.
	 *
	 * @code
	 *   RHI::GfxRenderBeginDesc beginDesc = {};
	 *   beginDesc.AddColorTarget(myColorTexture)
	 *            .SetDepth(myDepthTexture)
	 *            .SetSize(1920, 1080);
	 *   myCmdList->BeginRendering(beginDesc);
	 * @endcode
	 */
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
