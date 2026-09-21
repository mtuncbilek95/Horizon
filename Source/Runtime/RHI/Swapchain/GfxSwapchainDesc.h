#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Swapchain/GfxPresentMode.h>
#include <Runtime/RHI/Texture/GfxTextureFormat.h>

namespace Horizon::RHI
{
	constexpr u32 kMaxSwapchainImages = 8;

	struct GfxSwapchainDesc
	{
		void* pWindowHandle = nullptr;
		GfxTextureFormat format = GfxTextureFormat::RGBA8_UNORM;
		GfxPresentMode presentMode = GfxPresentMode::Immediate;
		u32 width = 0;
		u32 height = 0;
		u32 imageCount = 3;
	};
}
