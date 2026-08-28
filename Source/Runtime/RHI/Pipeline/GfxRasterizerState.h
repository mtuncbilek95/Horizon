#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Pipeline/GfxCullMode.h>
#include <Runtime/RHI/Pipeline/GfxFillMode.h>
#include <Runtime/RHI/Pipeline/GfxFrontFace.h>

namespace Horizon::RHI
{
	struct GfxRasterizerState
	{
		GfxFillMode fillMode = GfxFillMode::Solid;
		GfxCullMode cullMode = GfxCullMode::Back;
		GfxFrontFace frontFace = GfxFrontFace::CW;
		i32 depthBias = 0;
		f32 depthBiasClamp = 0.0f;
		f32 slopeScaledDepthBias = 0.0f;
	};
}
