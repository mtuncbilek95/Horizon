#pragma once

namespace Horizon
{
	enum class SamplerFilter
	{
		Nearest,
		Linear,
		Cubic
	};

	enum class SamplerMipMap
	{
		Nearest,
		Linear
	};

	enum class SamplerAddress
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder,
		MirrorClampToEdge
	};

	enum class BorderColor
	{
		TransparentBlack_Float,
		TransparentBlack_Int,
		OpaqueBlack_Float,
		OpaqueBlack_Int
	};
}
