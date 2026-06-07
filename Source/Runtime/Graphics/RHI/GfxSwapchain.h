#pragma once

#include <Runtime/Graphics/RHI/GfxObject.h>

#include <memory>
#include <vector>

namespace Horizon
{
	class GfxTexture;

	struct GfxSwapchainDesc
	{
		void* windowHandle = nullptr;
		u32 width = 0, height = 0;
		u32 imageCount = 2;
		GfxTextureFormat format = GfxTextureFormat::RGBA8;
		b8 vsync = true;
	};

	class GfxSwapchain
	{
	public:
		GfxSwapchain(const GfxSwapchainDesc& desc, GfxDevice* pDevice);
		~GfxSwapchain();

		GfxTexture* AcquireNext();
		void Present();
		void Resize(u32 width, u32 height);

	private:
		void CreateBackbuffers();

		GfxDevice* m_device = nullptr;
		GfxHandle m_native = nullptr;
		std::vector<std::unique_ptr<GfxTexture>> m_backbuffers;

		u32 m_imageCount = 2;
		u32 m_width = 0;
		u32 m_height = 0;
		GfxTextureFormat m_format = GfxTextureFormat::RGBA8;
		b8 m_vsync = true;
		b8 m_allowTearing = false;
		u32 m_backbufferIndex = 0;
	};
}
