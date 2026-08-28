#pragma once

#include <Runtime/RHI/Command/GfxLoadOp.h>
#include <Runtime/RHI/Command/GfxStoreOp.h>
#include <Runtime/RHI/Common/GfxColor.h>

namespace Horizon::RHI
{
	class GfxTexture;

	/**
	 * @brief One color target of a render pass, together
	 * with its load/store behaviour and the value used
	 * when the load operation is Clear.
	 *
	 * @code
	 *   RHI::GfxColorAttachment color = {};
	 *   color.pTexture = mySwapchainImage;
	 *   color.loadOp = RHI::GfxLoadOp::Clear;
	 *   color.clearColor = { 0.f, 0.f, 0.f, 1.f };
	 * @endcode
	 */
	struct GfxColorAttachment
	{
		GfxTexture* pTexture = nullptr;
		GfxLoadOp loadOp = GfxLoadOp::Clear;
		GfxStoreOp storeOp = GfxStoreOp::Store;
		GfxColor clearColor;
	};
}
