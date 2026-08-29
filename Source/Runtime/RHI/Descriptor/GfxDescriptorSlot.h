#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	class GfxDescriptorHeap;

	struct GfxDescriptorSlot
	{
		GfxDescriptorHeap* pHeap = nullptr;
		u32 index = kInvalid32;

		b8 IsValid() const { return pHeap != nullptr && index != kInvalid32; }
	};
}
