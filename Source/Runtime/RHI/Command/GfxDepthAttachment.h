#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Command/GfxLoadOp.h>
#include <Runtime/RHI/Command/GfxStoreOp.h>

namespace Horizon::RHI
{
	class GfxTexture;

	/**
	 * @brief The depth/stencil target of a render pass,
	 * together with its load/store behaviour and the
	 * values used when the load operation is Clear.
	 *
	 * @code
	 *   RHI::GfxDepthAttachment depth = {};
	 *   depth.pTexture = myDepthTexture;
	 *   depth.loadOp = RHI::GfxLoadOp::Clear;
	 *   depth.clearDepth = 1.0f;
	 * @endcode
	 */
	struct GfxDepthAttachment
	{
		GfxTexture* pTexture = nullptr;
		GfxLoadOp loadOp = GfxLoadOp::Clear;
		GfxStoreOp storeOp = GfxStoreOp::Store;
		f32 clearDepth = 1.0f;
		u8 clearStencil = 0;
	};
}
