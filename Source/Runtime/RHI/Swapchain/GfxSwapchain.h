#pragma once

#include <Runtime/Containers/List.h>
#include <Runtime/RHI/GfxTypes.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon
{
	class GfxTexture;
	class GfxQueue;
	class GfxFence;

	struct GfxSwapchainDesc
	{
		void* pWindowHandle = nullptr;

		u32 width = 0, height = 0;
		u32 imageCount = 3;
		GfxTextureFormat format = GfxTextureFormat::RGBA8;

		b8 vSync = true;
		b8 bAllowTearing = false;
	};

	class GfxSwapchain : public GfxObject
	{
	public:
		virtual GfxTexture* GetBackbuffer(u32 index) = 0;
		virtual b8 AcquireNextImage(GfxFence* pFence) = 0;
		virtual void Present(GfxQueue* pQueue, GfxFence* pFence) = 0;
		virtual void Resize(u32 width, u32 height) = 0;

		u32 GetCurrentIndex() const { return u32(m_currentIndex); }
		const GfxSwapchainDesc& GetDesc() const { return m_desc; }

	protected:
		GfxSwapchainDesc m_desc{};
		List<u64> m_imageFences;
		i8 m_currentIndex = -1;
	};
}