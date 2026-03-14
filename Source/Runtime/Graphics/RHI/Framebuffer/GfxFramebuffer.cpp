#include "GfxFramebuffer.h"

namespace Horizon
{
	GfxFramebuffer::GfxFramebuffer(const GfxFramebufferDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice), 
		m_desc(desc)
	{
	}
}
