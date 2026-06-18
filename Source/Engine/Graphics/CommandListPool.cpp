#include "CommandListPool.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>

namespace Horizon
{
	void CommandListPool::Init(GfxDevice* device, GfxQueueType type)
	{
		m_device = device;
		m_type = type;
	}

	GfxCommandList* CommandListPool::Acquire()
	{
		GfxCommandList* cmd = nullptr;
		{
			std::lock_guard lock(m_mutex);
			if (m_next == m_lists.size())
				m_lists.push_back(m_device->CreateCommandList(m_type));
			cmd = m_lists[m_next++].get();
		}

		cmd->Begin();
		return cmd;
	}

	void CommandListPool::Reset()
	{
		m_next = 0;
	}
}