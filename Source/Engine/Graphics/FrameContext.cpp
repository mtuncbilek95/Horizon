#include "FrameContext.h"

#include <Runtime/Graphics/RHI/Device/GfxDevice.h>
#include <Runtime/Graphics/RHI/Queue/GfxQueue.h>

namespace Horizon
{
	void FrameContext::Init(GfxDevice* device, GfxQueue* graphicsQueue)
	{
		m_graphicsQueue = graphicsQueue;
		m_fence = device->CreateFence();

		m_commandPools[u8(GfxQueueType::Graphics)].Init(device, GfxQueueType::Graphics);
		m_commandPools[u8(GfxQueueType::Compute)].Init(device, GfxQueueType::Compute);
		m_commandPools[u8(GfxQueueType::Transfer)].Init(device, GfxQueueType::Transfer);
	}

	void FrameContext::BeginFrame()
	{
		if (m_lastSignaled > 0)
			m_fence->WaitCPU(m_lastSignaled);

		for (CommandListPool& pool : m_commandPools)
			pool.Reset();
	}

	void FrameContext::EndFrame()
	{
		m_lastSignaled = m_graphicsQueue->Signal(m_fence.get());
		++m_frameIndex;
	}

	void FrameContext::WaitIdle()
	{
		if (m_lastSignaled > 0)
			m_fence->WaitCPU(m_lastSignaled);
	}

	GfxCommandList* FrameContext::AcquireCommandList(GfxQueueType type)
	{
		return m_commandPools[u8(type)].Acquire();
	}
}
