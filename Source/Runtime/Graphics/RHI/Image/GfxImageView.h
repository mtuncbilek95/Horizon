#pragma once

#include <Runtime/Graphics/RHI/Image/GfxImageViewDesc.h>
#include <Runtime/Graphics/RHI/Object/GfxObject.h>

namespace Horizon
{
	/**
	 * @class GfxImageView
	 * @brief GPU representation of the image buffers.
	 */
	class GfxImageView : public GfxObject
	{
	public:
		GfxImageView(const GfxImageViewDesc& desc, GfxDevice* pDevice);
		virtual ~GfxImageView() override = default;

		GfxImage* ImageOwner() const { return m_desc.image; }
		ImageViewType ViewType() const { return m_desc.viewType; }
		ImageAspect Aspect() const { return m_desc.aspect; }
		u32 BaseMip() const { return m_desc.baseMip; }
		u32 BaseArray() const { return m_desc.baseArray; }
		
		virtual void* Image() const = 0;
		virtual void* View() const = 0;

	private:
		GfxImageViewDesc m_desc;
	};
}
