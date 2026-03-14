#include "GfxFence.h"

namespace Horizon
{
	GfxFence::GfxFence(const GfxFenceDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice), 
		m_desc(desc)
	{
	}
}
