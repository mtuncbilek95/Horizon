#pragma once

#include <Runtime/RHI/Command/GfxLoadOp.h>
#include <Runtime/RHI/Command/GfxStoreOp.h>
#include <Runtime/RHI/Common/GfxColor.h>

namespace Horizon::RHI
{
	class GfxTexture;

	struct GfxColorAttachment
	{
		GfxTexture* pTexture = nullptr;
		GfxLoadOp loadOp = GfxLoadOp::Clear;
		GfxStoreOp storeOp = GfxStoreOp::Store;
		GfxColor clearColor;
	};
}
