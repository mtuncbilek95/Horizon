#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
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
