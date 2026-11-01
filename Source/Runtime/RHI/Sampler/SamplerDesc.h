#pragma once

#include <Runtime/RHI/Pipeline/CompareOp.h>
#include <Runtime/RHI/Sampler/SamplerAddressMode.h>
#include <Runtime/RHI/Sampler/SamplerBorderColor.h>
#include <Runtime/RHI/Sampler/SamplerFilter.h>

namespace Horizon::RHI
{
	struct SamplerDesc final
	{
		SamplerFilter minFilter = SamplerFilter::Linear;
		SamplerFilter magFilter = SamplerFilter::Linear;
		SamplerFilter mipFilter = SamplerFilter::Linear;

		SamplerAddressMode addressU = SamplerAddressMode::Repeat;
		SamplerAddressMode addressV = SamplerAddressMode::Repeat;
		SamplerAddressMode addressW = SamplerAddressMode::Repeat;

		f32 mipLodBias = 0.0f;
		f32 minLod = 0.0f;
		f32 maxLod = 1000.0f;
		u32 maxAnisotropy = 1;

		b8 compareEnable = false;
		CompareOp compareOp = CompareOp::Never;

		SamplerBorderColor borderColor = SamplerBorderColor::TransparentBlack;
	};
}
