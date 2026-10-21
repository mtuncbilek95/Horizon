#pragma once

#include <Engine/Asset/Texture/TextureAssetUsage.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/Math/Vec2u.h>
#include <Runtime/RHI/Texture/GfxTextureFormat.h>

namespace Horizon::Engine
{
	struct TextureAssetMip final
	{
		u64 offset = 0;
		u64 byteSize = 0;
		u32 width = 0;
		u32 height = 0;
	};

	struct TextureAssetProperties final
	{
		RHI::GfxTextureFormat format = RHI::GfxTextureFormat::Undefined;
		TextureAssetUsage assetUsage = TextureAssetUsage::Color;
		Math::Vec2u textureSize;
		u32 mipCount = 0;
		u64 textureByteSize = 0;
	};
}