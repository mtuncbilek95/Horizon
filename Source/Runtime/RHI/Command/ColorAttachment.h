#pragma once

#include <Runtime/RHI/Command/LoadOp.h>
#include <Runtime/RHI/Command/StoreOp.h>

namespace Horizon::RHI
{
	class TextureView;

	struct ColorAttachment final
	{
		TextureView* pView = nullptr;
		LoadOp loadOp = LoadOp::Clear;
		StoreOp storeOp = StoreOp::Store;
		f32 clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	};
}
