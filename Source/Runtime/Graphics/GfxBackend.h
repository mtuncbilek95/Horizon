#pragma once

namespace Horizon
{
	struct GfxDevice;
	struct GfxSwapchain;
	struct GfxQueue;
	struct GfxBuffer;
	struct GfxTexture;
	struct GfxPipeline;
	struct GfxCommandAllocator;
	struct GfxCommandList;
	struct GfxSync;

	struct GfxDeviceDesc
	{
		b8 enableDebug = true;
		b8 enableGPUValidation = false;

		GfxDeviceDesc& setDebug(b8 val) { enableDebug = val; return *this; }
		GfxDeviceDesc& setGPUValidation(b8 val) { enableGPUValidation = val; return *this; }
	};

	struct GfxSwapchainDesc
	{
		void* pWindowHandle = nullptr;
		u32 width = 0, height = 0;
		u32 imageCount = 2;
		b8 vSync = true;

		GfxSwapchainDesc& setWindowHandle(void* pHandl) { pWindowHandle = pHandl; return *this; }
		GfxSwapchainDesc& setImgSize(u32 w, u32 h) { width = w; height = h; return *this; }
		GfxSwapchainDesc& setImgCount(u32 count) { imageCount = count; return *this; }
		GfxSwapchainDesc& setSync(b8 val) { vSync = val; return *this; }
	};

	namespace Gfx
	{
		GfxDevice* CreateGfxDevice(const GfxDeviceDesc& desc);
		void ShutdownGfxDevice(GfxDevice* devHandl);

		GfxSwapchain* CreateGfxSwapchain(GfxDevice* pContext, const GfxSwapchainDesc& desc);
		void DestroyGfxSwapchain(GfxSwapchain* scHandl);
	}
}