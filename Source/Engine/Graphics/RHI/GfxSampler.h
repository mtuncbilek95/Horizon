#pragma once

#include <Engine/Graphics/RHI/GfxDevice.h>

#include <string_view>

namespace Horizon
{
	struct GfxSamplerDesc
	{
		GfxFilter minFilter = GfxFilter::Linear;
		GfxFilter magFilter = GfxFilter::Linear;
		GfxFilter mipFilter = GfxFilter::Linear;

		GfxAddressMode addressU = GfxAddressMode::Repeat;
		GfxAddressMode addressV = GfxAddressMode::Repeat;
		GfxAddressMode addressW = GfxAddressMode::Repeat;

		u32 maxAnisotropy = 1;
		b8 compareEnable = false;
		GfxCompareOp compareOp = GfxCompareOp::Never;
		GfxBorderColor border = GfxBorderColor::TransparentBlack;

		f32 minLod = 0.0f;
		f32 maxLod = 1000.0f;

		std::string_view debugName;
	};
}