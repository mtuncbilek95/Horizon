#pragma once

#include <Engine/Graphics/RHI/GfxDevice.h>

#include <glm/glm.hpp>

#include <string_view>

namespace Horizon
{
	struct GfxTextureDesc
	{
		u32 width = 1, height = 1;
		u32 depth = 1;
		u32 mipLevels = 1;

		GfxTextureFormat format = GfxTextureFormat::RGBA8;
		GfxTextureUsage usage = GfxTextureUsage::Sampled;
		GfxTextureType type = GfxTextureType::Tex2D;

		glm::vec4 clearColor = { 0.f, 0.f, 0.f, 1.f };
		f32 clearDepth = 0.0f;
		u8 clearStencil = 0;

		std::string_view debugName;
	};
}