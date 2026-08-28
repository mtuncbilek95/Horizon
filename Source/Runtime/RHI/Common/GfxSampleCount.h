#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Multisample count of a texture and of the
	 * pipeline that renders into it. Both sides have to
	 * agree or the pipeline is rejected.
	 *
	 * @code
	 *   RHI::GfxTextureDesc texDesc = {};
	 *   texDesc.sampleCount = RHI::GfxSampleCount::Count4;
	 * @endcode
	 */
	enum class GfxSampleCount : u8
	{
		Count1 = 1,
		Count2 = 2,
		Count4 = 4,
		Count8 = 8
	};
}