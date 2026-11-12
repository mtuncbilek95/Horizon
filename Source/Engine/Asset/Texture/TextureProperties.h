#pragma once

#include <Engine/Asset/Texture/TextureUsage.h>
#include <Runtime/RHI/Texture/TextureFormat.h>

namespace Horizon::Asset
{
	struct TextureMip
	{
		u64 offset = 0;
		u64 size = 0;
		u32 width = 0;
		u32 height = 0;
		u32 rowPitch = 0;
		u32 reserved = 0;
	};

	struct TextureProperties
	{
		RHI::TextureFormat format = RHI::TextureFormat::Undefined;
		TextureUsage usage = TextureUsage::Color;
		u32 width = 0;
		u32 height = 0;
		u32 mipCount = 0;
	};
}