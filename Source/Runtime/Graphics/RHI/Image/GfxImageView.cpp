#include "GfxImageView.h"

namespace Horizon
{
	GfxImageView::GfxImageView(const GfxImageViewDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice), 
		m_desc(desc)
	{
	}
}
