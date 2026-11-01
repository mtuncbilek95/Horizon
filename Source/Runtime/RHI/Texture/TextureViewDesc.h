#pragma once

#include <Runtime/RHI/Texture/TextureAspect.h>
#include <Runtime/RHI/Texture/TextureFormat.h>
#include <Runtime/RHI/Texture/TextureViewType.h>

namespace Horizon::RHI
{
	class Texture;

	struct TextureViewDesc final
	{
		Texture* pTexture = nullptr;
		TextureViewType type = TextureViewType::Texture2D;
		TextureFormat format = TextureFormat::Undefined;
		TextureAspect aspect = TextureAspect::All;

		u32 baseMip = 0;
		u32 mipCount = kInvalid32;
		u32 baseLayer = 0;
		u32 layerCount = kInvalid32;
	};
}