#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Dimensionality of a texture. Cube maps are
	 * 2D arrays instead of a separate type and are marked
	 * by the isCube flag of the descriptor.
	 *
	 * @code
	 *   RHI::GfxTextureDesc texDesc = {};
	 *   texDesc.type = RHI::GfxTextureType::Tex3D;
	 *   texDesc.depth = 64;
	 * @endcode
	 */
	enum class GfxTextureType : u8
	{
		Tex1D,
		Tex2D,
		Tex3D
	};
}