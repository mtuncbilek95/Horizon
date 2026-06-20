#pragma once

#include <Runtime/RHI/GfxTypes.h>
#include <Runtime/RHI/Object/GfxResource.h>

namespace Horizon
{
	class GfxFence
	{
	public:
		virtual ~GfxFence() = default;

		virtual u64 GetCompletedValue() = 0;
		virtual void WaitCPU(u64 value) = 0;
	};
}