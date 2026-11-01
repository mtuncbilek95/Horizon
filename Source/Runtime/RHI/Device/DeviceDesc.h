#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	struct DeviceDesc final
	{
		b8 enableDebug = false;
		b8 enableGpuValidation = false;
		b8 synchronizedCommandValidation = false;
	};
}