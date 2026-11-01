#pragma once

#include <Engine/Asset/Texture/TextureAssetUsage.h>
#include <Runtime/RHI/Texture/TextureFormat.h>

namespace Horizon::Engine
{
	struct TextureAssetMip
	{
		u64 offset = 0;
		u64 size = 0;
		u32 width = 0;
		u32 height = 0;
		u32 rowPitch = 0;
	};

	struct TextureAssetProperties
	{
		RHI::TextureFormat format = RHI::TextureFormat::Undefined;
		TextureAssetUsage usage = TextureAssetUsage::Color;
		u32 width = 0;
		u32 height = 0;
		u32 mipCount = 0;
	};
}