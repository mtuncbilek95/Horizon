#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Pipeline/GfxCullMode.h>
#include <Runtime/RHI/Pipeline/GfxFillMode.h>
#include <Runtime/RHI/Pipeline/GfxFrontFace.h>

namespace Horizon::RHI
{
	/**
	 * @brief Rasterizer setup of a graphics pipeline,
	 * covering fill mode, culling and the depth bias used
	 * mostly by shadow passes.
	 *
	 * @code
	 *   RHI::GfxRasterizerState raster = {};
	 *   raster.cullMode = RHI::GfxCullMode::None;
	 *   raster.depthBias = 100;
	 * @endcode
	 */
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
