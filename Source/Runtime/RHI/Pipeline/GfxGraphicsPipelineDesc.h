#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

#include <Runtime/RHI/Common/GfxSampleCount.h>
#include <Runtime/RHI/Pipeline/GfxBlendState.h>
#include <Runtime/RHI/Pipeline/GfxDepthStencilState.h>
#include <Runtime/RHI/Pipeline/GfxPrimitiveTopology.h>
#include <Runtime/RHI/Pipeline/GfxRasterizerState.h>
#include <Runtime/RHI/Texture/GfxTextureFormat.h>

namespace Horizon::RHI
{
	class GfxShader;

	/**
	 * @brief Creation descriptor of a graphics pipeline.
	 * Either the vertex and pixel pair or the task and
	 * mesh pair is filled, and the attachment formats have
	 * to match the render pass it will be used in.
	 *
	 * @code
	 *   RHI::GfxGraphicsPipelineDesc pipeDesc = {};
	 *   pipeDesc.pVertexShader = myVertexShader;
	 *   pipeDesc.pPixelShader = myPixelShader;
	 *   pipeDesc.colorFormats[0] = RHI::GfxTextureFormat::RGBA8_UNORM;
	 *   pipeDesc.colorTargetCount = 1;
	 *   RHI::GfxPipeline* pipeline = myRhiDevice->CreatePipeline(pipeDesc);
	 * @endcode
	 */
	struct GfxGraphicsPipelineDesc
	{
		GfxShader* pTaskShader = nullptr;
		GfxShader* pMeshShader = nullptr;
		GfxShader* pVertexShader = nullptr;
		GfxShader* pPixelShader = nullptr;

		GfxTextureFormat colorFormats[8] = {};
		u32 colorTargetCount = 0;
		GfxTextureFormat depthFormat = GfxTextureFormat::Undefined;

		GfxPrimitiveTopology topology = GfxPrimitiveTopology::TriangleList;
		GfxRasterizerState rasterizer;
		GfxDepthStencilState depthStencil;

		GfxBlendState blends[8];
		b8 independentBlend = false;

		GfxSampleCount sampleCount = GfxSampleCount::Count1;
	};
}
