#pragma once

#include <Runtime/RHI/Queue/GfxQueueType.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon::RHI
{
	class GfxCommandList;
	class GfxFence;

	/**
	 * @brief Hardware queue that consumes recorded command
	 * lists. Submission is asynchronous, so a fence is the
	 * only way to know when the work is done.
	 *
	 * @code
	 *   RHI::GfxQueue* queue = nullptr;
	 *   queue = myRhiDevice->CreateQueue(RHI::GfxQueueType::Graphics);
	 *   queue->Submit(&myCmdList, 1);
	 *   u64 value = queue->Signal(myFence);
	 * @endcode
	 */
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