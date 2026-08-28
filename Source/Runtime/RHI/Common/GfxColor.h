#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Straight RGBA color in linear floating
	 * point, used for attachment clear values and for
	 * blend constants.
	 *
	 * @code
	 *   RHI::GfxColor clear = { 0.1f, 0.1f, 0.1f, 1.0f };
	 * @endcode
	 */
	struct GfxColor
	{
		f32 r = 0.0f;
		f32 g = 0.0f;
		f32 b = 0.0f;
		f32 a = 1.0f;
	};
}