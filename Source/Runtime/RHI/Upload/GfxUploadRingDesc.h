#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Creation descriptor of a GfxUploadRing. The
	 * capacity has to cover the staging traffic of every
	 * frame that can be in flight at once.
	 *
	 * @code
	 *   RHI::GfxUploadRingDesc ringDesc = {};
	 *   ringDesc.capacity = 32 * 1024 * 1024;
	 * @endcode
	 */
	struct GfxUploadRingDesc
	{
		usize capacity = 0;
	};
}
