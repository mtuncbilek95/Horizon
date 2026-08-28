#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Which stage family a GfxPipeline was built
	 * for, and therefore which command list calls are
	 * legal after it is bound.
	 *
	 * @code
	 *   if (myPipeline->GetType() == RHI::GfxPipelineType::Compute)
	 *   {
	 *       myCmdList->Dispatch(8, 8, 1);
	 *   }
	 * @endcode
	 */
	enum class GfxPipelineType : u8
	{
		Graphics,
		Compute,
		Raytracing
	};
}