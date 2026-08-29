#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RHI/Descriptor/GfxDescriptorHeapType.h>

namespace Horizon::RHI
{
	struct GfxDescriptorHeapDesc
	{
		GfxDescriptorHeapType type = GfxDescriptorHeapType::Resource;
		u32 capacity = 0;
		u32 framesInFlight = 3;
		b8 shaderVisible = false;
	};
}
