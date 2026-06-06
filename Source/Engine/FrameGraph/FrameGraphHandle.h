#pragma once

#include <Engine/Graphics/RHI/GfxDevice.h>

namespace Horizon
{
	struct FGTextureTag {};
	using FGTextureHandle = Handle<FGTextureTag>;

	struct FGBufferTag {};
	using FGBufferHandle = Handle<FGBufferTag>;

	enum class FrameGraphResourceType : u8
	{
		Texture,
		Buffer
	};

	enum class FrameGraphQueue
	{
		Graphics,
		Compute
	};
}