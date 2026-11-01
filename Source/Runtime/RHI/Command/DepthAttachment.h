#pragma once

#include <Runtime/RHI/Command/LoadOp.h>
#include <Runtime/RHI/Command/StoreOp.h>

namespace Horizon::RHI
{
	class TextureView;

	struct DepthAttachment final
	{
		TextureView* pView = nullptr;
		LoadOp loadOp = LoadOp::Clear;
		StoreOp storeOp = StoreOp::Store;
		f32 clearDepth = 1.0f;
		u8 clearStencil = 0;
	};
}
