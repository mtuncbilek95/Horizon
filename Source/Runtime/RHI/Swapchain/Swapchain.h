#pragma once

#include <Runtime/Containers/Memory/UniquePtr.h>
#include <Runtime/RHI/Object/Object.h>
#include <Runtime/RHI/Swapchain/SwapchainDesc.h>
#include <Runtime/RHI/Texture/Texture.h>
#include <Runtime/RHI/Texture/TextureView.h>

namespace Horizon::RHI
{
	class Swapchain : public Object
	{
	public:
		virtual b8 AcquireNextImage() = 0;
		virtual void Present() = 0;
		virtual void Resize(u32 width, u32 height) = 0;

		Texture* GetImage(u32 index) const;
		TextureView* GetImageView(u32 index) const;

		Texture* GetCurrentImage() const { return m_images[m_imageIndex].Get(); }
		TextureView* GetCurrentImageView() const { return m_views[m_imageIndex].Get(); }

		u32 GetCurrentImageIndex() const { return m_imageIndex; }
		u32 GetImageCount() const { return m_imageCount; }

		u32 GetWidth() const { return m_width; }
		u32 GetHeight() const { return m_height; }
		TextureFormat GetFormat() const { return m_format; }
		PresentMode GetPresentMode() const { return m_presentMode; }

	protected:
		UniquePtr<Texture> m_images[kMaxSwapchainImages];
		UniquePtr<TextureView> m_views[kMaxSwapchainImages];

		TextureFormat m_format = TextureFormat::BGRA8_UNORM;
		PresentMode m_presentMode = PresentMode::Vsync;

		u32 m_width = 0;
		u32 m_height = 0;
		u32 m_imageCount = 0;
		u32 m_imageIndex = 0;
	};
}
