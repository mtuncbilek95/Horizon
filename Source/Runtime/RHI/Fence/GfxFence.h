#pragma once

#include <Runtime/RHI/GfxTypes.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon
{
	class GfxFence : public GfxObject
	{
	public:
		virtual u64 GetCompletedValue() = 0;
		virtual void WaitCPU(u64 value) = 0;

		b8 IsComplete(u64 value) { return GetCompletedValue() >= value; }
	};
}