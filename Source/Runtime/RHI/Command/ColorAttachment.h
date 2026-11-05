#pragma once

#include <Runtime/RHI/Command/LoadOp.h>
#include <Runtime/RHI/Command/StoreOp.h>
#include <Runtime/Math/Color4f.h>

namespace Horizon::RHI
{
	class TextureView;

	struct ColorAttachment final
	{
		TextureView* pView = nullptr;
		LoadOp loadOp = LoadOp::Clear;
		StoreOp storeOp = StoreOp::Store;
		Math::Color4f clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	};
}
