#pragma once

#include <Runtime/RHI/Common/SampleCount.h>
#include <Runtime/RHI/Texture/TextureFormat.h>
#include <Runtime/RHI/Texture/TextureType.h>
#include <Runtime/RHI/Texture/TextureUsage.h>

namespace Horizon::RHI
{
	struct TextureDesc final
	{
		TextureType type = TextureType::Texture2D;
		TextureFormat format = TextureFormat::Undefined;
		TextureUsage usage = TextureUsage::None;

		u32 width = 1;
		u32 height = 1;
		u32 depth = 1;
		u32 arrayLayers = 1;
		u32 mipLevels = 1;

		SampleCount sampleCount = SampleCount::Count1;

		f32 clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		f32 clearDepth = 1.0f;
		u8 clearStencil = 0;
	};
}