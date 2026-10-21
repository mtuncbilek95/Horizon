#pragma once

#include <Runtime/RHI/Texture/GfxTextureFormat.h>

namespace Horizon::Engine
{
	struct ForwardPassFormats final
	{
		static constexpr RHI::GfxTextureFormat Color = RHI::GfxTextureFormat::RGBA8_UNORM;
		static constexpr RHI::GfxTextureFormat Depth = RHI::GfxTextureFormat::D32_FLOAT;
	};
}