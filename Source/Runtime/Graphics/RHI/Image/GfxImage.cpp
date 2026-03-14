#include "GfxImage.h"

namespace Horizon
{
	GfxImage::GfxImage(const GfxImageDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice), 
		m_desc(desc), m_isSwapchain(false)
	{
	}

	GfxImage::GfxImage(const GfxImageDesc& desc, GfxDevice* pDevice, void* apiImage) : GfxObject(pDevice),
		m_desc(desc), m_isSwapchain(true)
	{
	}
}
