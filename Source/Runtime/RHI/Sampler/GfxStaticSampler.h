#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	/**
	 * @brief The immutable sampler set baked into every
	 * pipeline. Shaders reach them by this index, so no
	 * sampler descriptor has to be allocated per material.
	 *
	 * @code
	 *   // HLSL side
	 *   //   SamplerState samplers[] : register(s0, space1);
	 *   //   float4 c = tex.Sample(samplers[LinearWrap], uv);
	 * @endcode
	 */
	enum class GfxStaticSampler : u8
	{
		PointClamp,
		PointWrap,
		LinearClamp,
		LinearWrap,
		LinearMirror,
		AnisoClamp,
		AnisoWrap,
		ShadowCompare,
		RevShadowCompare,
		Count
	};
}