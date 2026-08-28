#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Swapchain/GfxPresentMode.h>
#include <Runtime/RHI/Texture/GfxTextureFormat.h>

namespace Horizon::RHI
{
	class GfxDescriptorHeap;

	constexpr u32 kMaxSwapchainImages = 8;

	/**
	 * @brief Creation descriptor of a GfxSwapchain. The
	 * color heap is needed because every backbuffer gets a
	 * render target view allocated from it.
	 *
	 * @code
	 *   RHI::GfxSwapchainDesc chainDesc = {};
	 *   chainDesc.pWindowHandle = myWindow->GetNativeHandle();
	 *   chainDesc.pColorHeap = myColorHeap;
	 *   chainDesc.width = 1920;
	 *   chainDesc.height = 1080;
	 * @endcode
	 */
	struct GfxSwapchainDesc
	{
		void* pWindowHandle = nullptr;
		GfxDescriptorHeap* pColorHeap = nullptr;

		GfxTextureFormat format = GfxTextureFormat::RGBA8_UNORM;
		GfxPresentMode presentMode = GfxPresentMode::Immediate;

		u32 width = 0;
		u32 height = 0;
		u32 imageCount = 3;
	};
}
