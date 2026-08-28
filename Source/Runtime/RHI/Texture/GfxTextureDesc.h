#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Common/GfxSampleCount.h>
#include <Runtime/RHI/Common/GfxColor.h>
#include <Runtime/RHI/Texture/GfxTextureFormat.h>
#include <Runtime/RHI/Texture/GfxTextureType.h>
#include <Runtime/RHI/Texture/GfxTextureUsage.h>

namespace Horizon::RHI
{
	/**
	 * @brief Creation descriptor of a GfxTexture. The
	 * usage flags decide which views are created, and the
	 * clear color is the one the render pass may load.
	 *
	 * @code
	 *   RHI::GfxTextureDesc texDesc = {};
	 *   texDesc.format = RHI::GfxTextureFormat::RGBA8_UNORM;
	 *   texDesc.usage = RHI::GfxTextureUsage::Sampled | RHI::GfxTextureUsage::TransferDst;
	 *   texDesc.width = 1024;
	 *   texDesc.height = 1024;
	 * @endcode
	 */
	struct GfxTextureDesc
	{
		GfxTextureType type = GfxTextureType::Tex2D;
		GfxTextureFormat format = GfxTextureFormat::Undefined;
		GfxTextureUsage usage = GfxTextureUsage::Sampled;

		u32 width = 1;
		u32 height = 1;
		u32 depth = 1;
		u32 arraySize = 1;
		u32 mipLevels = 1;

		GfxColor clearColor = {};

		GfxSampleCount sampleCount = GfxSampleCount::Count1;
		b8 isCube = false;
	};
}
