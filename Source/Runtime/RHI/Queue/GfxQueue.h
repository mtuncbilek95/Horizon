#pragma once

#include <Runtime/RHI/Queue/GfxQueueType.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon::RHI
{
	class GfxCommandList;
	class GfxFence;

	class GfxQueue : public GfxObject
	{
	public:
		virtual void Submit(GfxCommandList* const* ppLists, u32 count) = 0;
		virtual u64 Signal(GfxFence* pFence) = 0;
		virtual void Wait(GfxFence* pFence, u64 value) = 0;
		GfxQueueType GetType() const { return m_type; }
	protected:
		GfxQueueType m_type = GfxQueueType::Graphics;
	};
}