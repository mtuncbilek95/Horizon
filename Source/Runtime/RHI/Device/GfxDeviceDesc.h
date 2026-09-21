#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	struct GfxDeviceDesc
	{
		b8 enableDebugLayer = false;
		b8 enableGpuValidation = false;
		b8 synchronizedCommandValidation = false;

		u32 resourceDescriptorCapacity = 1 << 17;
		u32 samplerDescriptorCapacity = 2048;
		u32 colorDescriptorCapacity = 1 << 10;
		u32 depthDescriptorCapacity = 1 << 8;
	};
}