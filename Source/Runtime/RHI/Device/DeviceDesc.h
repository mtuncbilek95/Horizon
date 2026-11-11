#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RHI/Backend.h>

namespace Horizon::RHI
{
	struct DeviceDesc final
	{
		Backend api = Backend::DX12;
		b8 enableDebug = false;
		b8 enableGpuValidation = false;
		b8 synchronizedCommandValidation = false;
	};
}