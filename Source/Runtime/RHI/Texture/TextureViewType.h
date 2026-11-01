#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	enum class TextureViewType : u8
	{
		Texture1D,
		Texture1DArray,
		Texture2D,
		Texture2DArray,
		Texture3D,
		TextureCube,
		TextureCubeArray
	};
}