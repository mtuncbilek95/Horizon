#pragma once

#include <Engine/Graphics/RHI/GfxDevice.h>

#include <span>

namespace Horizon
{
	struct GfxWaitInfo
	{
		GfxSemaphore semaphore;
		u64 value;
	};

	struct GfxSignalInfo
	{
		GfxSemaphore semaphore;
		u64 value;
	};

	struct GfxSubmitInfo
	{
		std::span<GfxCommandList*> lists;
		std::span<const GfxWaitInfo> waits;
		std::span<const GfxSignalInfo> signals;
	};

	namespace GfxQueue
	{
		void Submit(GfxQueueType type, const GfxSubmitInfo& info);
	}
}