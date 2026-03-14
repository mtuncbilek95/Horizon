#include "GfxCommandPool.h"

namespace Horizon
{
	GfxCommandPool::GfxCommandPool(const GfxCommandPoolDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice),
		m_desc(desc)
	{
	}
}
