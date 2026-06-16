#include "GfxDevice.h"

#include <Runtime/Graphics/RHI/GfxTypes.h>

namespace Horizon
{
	void GfxDevice::EnqueueDelete(GfxResource* pResource)
	{
		std::lock_guard lock(m_mutex);
		m_pending.push({ pResource, m_frameIndex.load() });
	}

	void GfxDevice::FlushPendingDeletes(u64 currentFrame)
	{
		std::lock_guard lock(m_mutex);
		while (!m_pending.empty() && m_pending.front().retireFrame + MaxFramesInFlight <= currentFrame)
		{
			delete m_pending.front().pResource;
			m_pending.pop();
		}
	}

	void GfxDevice::SetFrameIndex(u64 idx)
	{
		m_frameIndex = idx;
	}
}