#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Bitwise channel mask of the render target
	 * write. Channels left out of the mask keep whatever
	 * the target already held.
	 *
	 * @code
	 *   RHI::GfxBlendState blend = {};
	 *   blend.writeMask = RHI::GfxColorWrite::Red | RHI::GfxColorWrite::Alpha;
	 * @endcode
	 */
	enum class GfxColorWrite : u8
	{
		None = 0,

		Red = 1 << 0,
		Green = 1 << 1,
		Blue = 1 << 2,
		Alpha = 1 << 3,

		All = Red | Green | Blue | Alpha
	};
}
