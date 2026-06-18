#pragma once

#include <Runtime/Graphics/RHI/Fence/GfxFence.h>
#include <Engine/Graphics/CommandListPool.h>

#include <memory>
#include <array>

namespace Horizon
{
	class GfxDevice;
	class GfxQueue;
	class GfxCommandList;

	class FrameContext
	{
	public:
		void Init(GfxDevice* device, GfxQueue* graphicsQueue);

		void BeginFrame();
		void EndFrame();
		void WaitIdle();

		GfxCommandList* AcquireCommandList(GfxQueueType type = GfxQueueType::Graphics);
		u64 GetFrameIndex() const { return m_frameIndex; }

	private:
		static constexpr usize QueueTypeCount = 3;

		GfxQueue* m_graphicsQueue = nullptr;
		std::unique_ptr<GfxFence> m_fence;
		std::array<CommandListPool, usize(GfxQueueType::Count)> m_commandPools;

		u64 m_lastSignaled = 0;
		u64 m_frameIndex = 0;
	};
}
