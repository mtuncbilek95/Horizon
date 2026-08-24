#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
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