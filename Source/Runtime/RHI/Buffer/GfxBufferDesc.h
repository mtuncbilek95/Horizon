#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Buffer/GfxBufferUsage.h>
#include <Runtime/RHI/Memory/GfxMemoryType.h>

namespace Horizon::RHI
{
	struct GfxBufferDesc
	{
		GfxBufferUsage usage = GfxBufferUsage::None;
		GfxMemoryType memory = GfxMemoryType::GpuOnly;
		usize size = 0;
		u32 stride = 0;
	};
}
