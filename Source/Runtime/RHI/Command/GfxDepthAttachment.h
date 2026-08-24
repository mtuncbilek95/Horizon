#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Command/GfxLoadOp.h>
#include <Runtime/RHI/Command/GfxStoreOp.h>

namespace Horizon::RHI
{
	class GfxTexture;

	struct GfxDepthAttachment
	{
		GfxTexture* pTexture = nullptr;
		GfxLoadOp loadOp = GfxLoadOp::Clear;
		GfxStoreOp storeOp = GfxStoreOp::Store;
		f32 clearDepth = 1.0f;
		u8 clearStencil = 0;
	};
}
