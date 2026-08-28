#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Buffer/GfxBufferUsage.h>
#include <Runtime/RHI/Memory/GfxMemoryType.h>

namespace Horizon::RHI
{
	struct GfxBufferArenaDesc
	{
		GfxBufferUsage usage = GfxBufferUsage::Storage;
		GfxMemoryType memory = GfxMemoryType::GpuOnly;
		usize capacity = 0;
	};
}
