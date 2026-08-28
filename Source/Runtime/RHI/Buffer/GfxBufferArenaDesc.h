#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Buffer/GfxBufferUsage.h>
#include <Runtime/RHI/Memory/GfxMemoryType.h>

namespace Horizon::RHI
{
	/**
	 * @brief Creation descriptor of a GfxBufferArena. The
	 * usage and the memory type are inherited by the backing
	 * buffer, while the capacity fixes its total byte size.
	 *
	 * @code
	 *   RHI::GfxBufferArenaDesc arenaDesc = {};
	 *   arenaDesc.usage = RHI::GfxBufferUsage::Storage;
	 *   arenaDesc.capacity = 64 * 1024 * 1024;
	 * @endcode
	 */
	struct GfxBufferArenaDesc
	{
		GfxBufferUsage usage = GfxBufferUsage::Storage;
		GfxMemoryType memory = GfxMemoryType::GpuOnly;

		usize capacity = 0;
	};
}
