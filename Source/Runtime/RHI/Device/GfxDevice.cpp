#include "GfxDevice.h"

#include <Runtime/RHI/GfxTypes.h>
#include <Runtime/Definitions/Allocator.h>

namespace Horizon
{
	void GfxDevice::EnqueueDelete(GfxResource* pResource)
	{
		ScopedLock lock(m_mutex);
		m_pending.push({ pResource, m_frameIndex.Load() });
	}

	void GfxDevice::FlushPendingDeletes(u64 currentFrame)
	{
		ScopedLock lock(m_mutex);
		while (!m_pending.empty() && m_pending.front().retireFrame + MaxFramesInFlight <= currentFrame)
		{
			Allocator::Delete(m_pending.front().pResource);
			m_pending.pop();
		}
	}

	void GfxDevice::SetFrameIndex(u64 idx)
	{
		m_frameIndex.Store(idx);
	}
}