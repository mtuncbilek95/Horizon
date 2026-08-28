#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Object/GfxObject.h>
#include <Runtime/RHI/Swapchain/GfxSwapchainDesc.h>

namespace Horizon::RHI
{
	class GfxTexture;
	class GfxQueue;
	class GfxFence;

	/**
	 * @brief Chain of backbuffers owned by the window.
	 * A frame starts by acquiring an image and ends by
	 * presenting it on the queue given at creation.
	 *
	 * @code
	 *   myChain->AcquireNextImage(myFence);
	 *   RHI::GfxTexture* image = myChain->GetImage(myChain->GetCurrentImageIndex());
	 *   myChain->Present(myQueue, myFence);
	 * @endcode
	 */
	class GfxSwapchain : public GfxObject
	{
	public:
		virtual b8 AcquireNextImage(GfxFence* pFence) = 0;
		virtual void Present(GfxQueue* pQueue, GfxFence* pFence) = 0;
		virtual void Resize(u32 width, u32 height) = 0;

		virtual GfxTexture* GetImage(u32 index) const = 0;

		const GfxSwapchainDesc& GetDesc() const { return m_desc; }
		u32 GetImageCount() const { return m_desc.imageCount; }
		u32 GetCurrentImageIndex() const { return m_imageIndex; }

	protected:
		GfxSwapchainDesc m_desc{};
		u32 m_imageIndex = 0;
		u64 m_imageFenceValues[kMaxSwapchainImages] = {};
	};
}