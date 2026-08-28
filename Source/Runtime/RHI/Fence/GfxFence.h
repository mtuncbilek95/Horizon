#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon::RHI
{
	/**
	 * @brief Monotonic timeline value the GPU signals as
	 * it drains work. Used both to block the CPU and to
	 * order submissions between queues.
	 *
	 * @code
	 *   u64 value = myQueue->Signal(myFence);
	 *   myFence->WaitCPU(value);
	 * @endcode
	 */
	class GfxFence : public GfxObject
	{
	public:
		virtual u64 GetCompletedValue() = 0;
		virtual void WaitCPU(u64 value) = 0;

		b8 IsComplete(u64 value) { return GetCompletedValue() >= value; }
	};
}
