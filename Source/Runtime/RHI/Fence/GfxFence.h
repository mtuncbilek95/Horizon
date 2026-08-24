#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon::RHI
{
	class GfxFence : public GfxObject
	{
	public:
		virtual u64 GetCompletedValue() = 0;
		virtual void WaitCPU(u64 value) = 0;

		b8 IsComplete(u64 value) { return GetCompletedValue() >= value; }
	};
}
