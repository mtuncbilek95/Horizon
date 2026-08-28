#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	struct GfxDeviceDesc
	{
		b8 enableDebugLayer = false;
		b8 enableGpuValidation = false;
		b8 synchronizedCommandValidation = false;
	};
}