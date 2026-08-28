#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Heap a resource is allocated from, which
	 * decides whether the CPU can map it and in which
	 * direction the transfers are cheap.
	 *
	 * @code
	 *   RHI::GfxBufferDesc bufDesc = {};
	 *   bufDesc.memory = RHI::GfxMemoryType::Upload;
	 * @endcode
	 */
	enum class GfxMemoryType : u8
	{
		GpuOnly,
		Upload,
		GpuUpload,
		Readback
	};
}