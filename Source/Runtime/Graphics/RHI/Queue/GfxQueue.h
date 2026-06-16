#pragma once

#include <Runtime/Graphics/RHI/GfxTypes.h>
#include <Runtime/Graphics/RHI/Object/GfxResource.h>

namespace Horizon
{
	class GfxCommandList;
	class GfxFence;

	class GfxQueue
	{
	public:
		virtual ~GfxQueue() = default;

		virtual void Submit(GfxCommandList* const* lists, u32 count) = 0;
		virtual u64 Signal(GfxFence* fence) = 0;
		virtual void Wait(GfxFence* fence, u64 value) = 0;

		GfxQueueType GetType() const { return m_type; }
	protected:
		GfxQueueType m_type = GfxQueueType::Graphics;
	};
}