#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Buffer/GfxBufferUsage.h>
#include <Runtime/RHI/Memory/GfxMemoryType.h>

namespace Horizon::RHI
{
	/**
	 * @brief Creation descriptor of a GfxBuffer. Declares
	 * the usage flags, the memory heap it lives on, the
	 * total byte size and the per element stride.
	 *
	 * @code
	 *   RHI::GfxBufferDesc bufDesc = {};
	 *   bufDesc.usage = RHI::GfxBufferUsage::Storage;
	 *   bufDesc.memory = RHI::GfxMemoryType::GpuOnly;
	 *   bufDesc.size = sizeof(Vertex) * vertexCount;
	 *   bufDesc.stride = sizeof(Vertex);
	 * @endcode
	 */
	struct GfxBufferDesc
	{
		GfxBufferUsage usage = GfxBufferUsage::None;
		GfxMemoryType memory = GfxMemoryType::GpuOnly;

		usize size = 0;
		u32 stride = 0;
	};
}
