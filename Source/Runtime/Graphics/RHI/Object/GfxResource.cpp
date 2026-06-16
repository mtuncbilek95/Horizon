#include "GfxResource.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>

namespace Horizon
{
	void GfxResource::Release()
	{
		if (m_refCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
			m_ownerDevice->EnqueueDelete(this);
	}
}