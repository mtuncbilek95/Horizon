#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	class TextureView;
}

namespace Horizon::Asset
{
	class Texture;

	struct TextureUsePermit
	{
		Texture* pAsset = nullptr;
		RHI::TextureView* pView = nullptr;
		u32 viewIndex = kInvalid32;

		b8 IsValid() const { return pAsset != nullptr; }
	};
}