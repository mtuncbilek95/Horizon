#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class TextureAspect : u8
	{
		All,
		Depth,
		Stencil
	};
}