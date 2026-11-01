#pragma once

#include <Runtime/PAL/Window/Window.h>
#include <Runtime/RHI/Swapchain/PresentMode.h>
#include <Runtime/RHI/Texture/TextureFormat.h>

namespace Horizon::RHI
{
	constexpr u32 kMaxSwapchainImages = 8;

	struct SwapchainDesc final
	{
		PAL::OSHandle windowHandle = {};
		TextureFormat format = TextureFormat::BGRA8_UNORM;
		PresentMode presentMode = PresentMode::Vsync;
		u32 width = 0;
		u32 height = 0;
		u32 imageCount = 3;
	};
}
