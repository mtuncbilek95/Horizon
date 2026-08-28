#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Bitwise usage flags that declare how a
	 * GfxBuffer will be bound by the pipeline. Multiple
	 * flags can be combined for a single buffer.
	 *
	 * @code
	 *   RHI::GfxBufferDesc bufDesc = {};
	 *   bufDesc.usage = RHI::GfxBufferUsage::Storage |
	 *                   RHI::GfxBufferUsage::TransferDst;
	 * @endcode
	 */
	enum class GfxBufferUsage : u32
	{
		None = 0,

		Index = 1 << 0,
		Constant = 1 << 1,
		Storage = 1 << 2,
		Indirect = 1 << 3,
		TransferSrc = 1 << 4,
		TransferDst = 1 << 5,
		AccelerationStructure = 1 << 6
	};
}
