#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class GfxMemoryType : u8
	{
		GpuOnly,
		Upload,
		GpuUpload,
		Readback
	};
}