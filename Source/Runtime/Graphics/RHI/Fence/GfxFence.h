#pragma once

#include <Runtime/Graphics/RHI/GfxTypes.h>
#include <Runtime/Graphics/RHI/Object/GfxResource.h>

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