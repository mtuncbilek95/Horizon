#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Coefficient the source or the destination
	 * color is multiplied with before the blend operation
	 * combines the two.
	 *
	 * @code
	 *   RHI::GfxBlendState blend = {};
	 *   blend.srcColor = RHI::GfxBlendFactor::SrcAlpha;
	 *   blend.dstColor = RHI::GfxBlendFactor::InvSrcAlpha;
	 * @endcode
	 */
	enum class GfxBlendFactor : u8
	{
		Zero,
		One,
		SrcColor,
		InvSrcColor,
		SrcAlpha,
		InvSrcAlpha,
		DstColor,
		InvDstColor,
		DstAlpha,
		InvDstAlpha,
		SrcAlphaSaturate,
		ConstantColor,
		InvConstantColor
	};
}
