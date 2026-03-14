#include "GfxRenderPass.h"

namespace Horizon
{
	GfxRenderPass::GfxRenderPass(const GfxRenderPassDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice), 
		m_desc(desc)
	{
	}
}
