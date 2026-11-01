#pragma once

#include <Runtime/RHI/Command/ResourceState.h>

namespace Horizon::RHI
{
	class Texture;

	struct TextureBarrier final
	{
		Texture* pTexture = nullptr;
		ResourceState before = ResourceState::Undefined;
		ResourceState after = ResourceState::Undefined;

		u32 baseMip = 0;
		u32 mipCount = kInvalid32;
		u32 baseLayer = 0;
		u32 layerCount = kInvalid32;
	};
}
