#include "GfxQueue.h"

namespace Horizon
{
	GfxQueue::GfxQueue(const GfxQueueDesc& desc, GfxDevice* pDevice) : GfxObject(pDevice), m_desc(desc)
	{
	}
}
