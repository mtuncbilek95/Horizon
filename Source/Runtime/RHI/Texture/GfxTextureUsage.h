#pragma once

#include <Runtime/Definitions/BitwiseOperators.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief Bitwise usage flags that declare how a
	 * GfxTexture will be bound. Every flag costs a view,
	 * so only the ones actually needed should be set.
	 *
	 * @code
	 *   RHI::GfxTextureDesc texDesc = {};
	 *   texDesc.usage = RHI::GfxTextureUsage::RenderTarget | RHI::GfxTextureUsage::Sampled;
	 * @endcode
	 */
	enum class GfxTextureUsage : u32
	{
		None = 0,

		Sampled = 1 << 0,
		Storage = 1 << 1,
		RenderTarget = 1 << 2,
		DepthStencil = 1 << 3,
		TransferSrc = 1 << 4,
		TransferDst = 1 << 5
	};
}
