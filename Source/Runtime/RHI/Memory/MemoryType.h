#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class MemoryType : u8
	{
		GpuOnly,
		Upload,
		GpuUpload,
		Readback
	};
}