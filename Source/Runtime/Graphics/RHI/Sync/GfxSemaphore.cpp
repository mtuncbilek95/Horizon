#include "GfxSemaphore.h"

namespace Horizon
{
	GfxSemaphore::GfxSemaphore(const GfxSemaphoreDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice), 
		m_desc(desc)
	{
	}
}
