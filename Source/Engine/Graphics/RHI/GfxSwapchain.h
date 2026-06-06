#pragma once

#include <Engine/Graphics/RHI/GfxDevice.h>

namespace Horizon
{
	struct GfxSwapchainDesc
	{
		void* windowHandle = nullptr;
		u32 width = 0, height = 0;
		u32 imageCount = 2;
		GfxTextureFormat format = GfxTextureFormat::RGBA8;
		b8 vsync = true;
	};

	namespace GfxSwapchain
	{
		void Create(const GfxSwapchainDesc& desc);
		void Destroy();
		GfxTextureHandle AcquireNext();
		void Present();
		void Resize(u32 width, u32 height);
	}
}