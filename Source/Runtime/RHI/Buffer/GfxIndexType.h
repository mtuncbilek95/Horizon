#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Element width of an index buffer, used
	 * while binding it to the command buffer.
	 *
	 * @code
	 *   myCmdBuffer->BindIndexBuffer(idxBuf, RHI::GfxIndexType::Index32);
	 * @endcode
	 */
	enum class GfxIndexType : u8
	{
		Index16,
		Index32
	};
}
