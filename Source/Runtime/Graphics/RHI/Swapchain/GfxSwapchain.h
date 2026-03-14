#pragma once

#include <Runtime/Graphics/RHI/Image/GfxImageDesc.h>
#include <Runtime/Graphics/RHI/Object/GfxObject.h>
#include <Runtime/Graphics/RHI/Swapchain/GfxSwapchainDesc.h>

#include <memory>
#include <vector>

namespace Horizon
{
	class GfxImage;
	class GfxImageView;
	class GfxSemaphore;
	class GfxFence;

	/**
	 * @class GfxInstance
	 * @brief Backbuffer controller of the window. It generates and controls
	 * image buffers of the window unit.
	 */
	class GfxSwapchain : public GfxObject
	{
	public:
		GfxSwapchain(const GfxSwapchainDesc& desc, GfxDevice* pDevice);
		virtual ~GfxSwapchain() override = default;

		virtual u32 AcquireNextImage(GfxSemaphore* signal, GfxFence* fence) = 0;
		virtual void Present(const std::vector<GfxSemaphore*>& waits) const = 0;
		virtual void Resize(const Math::Vec2u& newSize);

		const Math::Vec2u& ImageSize() const { return m_desc.imageSize; }
		u8 ImageCount() const { return m_desc.imageCount; }
		ImageFormat ImageFormat() const { return m_desc.format; }
		PresentMode PresentMode() const { return m_desc.presentMode; }
		GfxQueue* GraphicsQueue() const { return m_desc.graphicsQueue; }

		GfxImage* Image(usize index) const { return m_images.at(index).get(); }
		GfxImageView* ImageView(usize index) const { return m_views.at(index).get(); }

	protected:
		void AddImage(std::shared_ptr<GfxImage> img) { m_images.push_back(img); }
		void AddView(std::shared_ptr<GfxImageView> view) { m_views.push_back(view); }
		void SetSize(const Math::Vec2u& size) { m_desc.setImageSize(size); }

	private:
		GfxSwapchainDesc m_desc;

		std::vector<std::shared_ptr<GfxImage>> m_images;
		std::vector<std::shared_ptr<GfxImageView>> m_views;
	};
}
