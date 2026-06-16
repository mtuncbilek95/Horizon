#pragma once

#include <Runtime/Graphics/RHI/GfxTypes.h>
#include <Runtime/Graphics/RHI/Object/GfxResource.h>

namespace Horizon
{
	class GfxTexture;

	struct GfxSwapchainDesc
	{
		void* pWindowHandle = nullptr;

		u32 width = 0;
		u32 height = 0;
		u32 imageCount = 2;

		b8 vSync = true;
		b8 bAllowTearing = false;
	};

	class GfxSwapchain
	{
	public:
		virtual ~GfxSwapchain() = default;

		virtual GfxTexture* GetBackbuffer(u32 index) = 0;
		virtual u32  GetCurrentIndex() = 0;
		virtual void Present() = 0;
		virtual void Resize(u32 width, u32 height) = 0;

	protected:
		u32 m_imageCount = 0;
	};
}