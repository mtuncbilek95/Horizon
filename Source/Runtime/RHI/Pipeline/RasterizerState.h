#pragma once

#include <Runtime/RHI/Pipeline/CullMode.h>
#include <Runtime/RHI/Pipeline/FillMode.h>
#include <Runtime/RHI/Pipeline/FrontFace.h>

namespace Horizon::RHI
{
	struct RasterizerState final
	{
		FillMode fillMode = FillMode::Solid;
		CullMode cullMode = CullMode::Back;
		FrontFace frontFace = FrontFace::CW;

		b8 depthClip = true;
		i32 depthBias = 0;
		f32 depthBiasClamp = 0.0f;
		f32 slopeScaledDepthBias = 0.0f;
	};
}
