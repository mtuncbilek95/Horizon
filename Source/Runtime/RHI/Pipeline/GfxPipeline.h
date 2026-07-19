#pragma once

#include <Runtime/RHI/GfxTypes.h>
#include <Runtime/RHI/Object/GfxObject.h>

namespace Horizon
{
	struct GfxShaderBlob
	{
		const void* pData = nullptr;
		usize size = 0;

		b8 IsValid() const { return pData != nullptr && size > 0; }
	};

	struct GfxRasterizerState
	{
		GfxFillMode fillMode = GfxFillMode::Solid;
		GfxCullMode cullMode = GfxCullMode::Back;
		GfxFrontFace frontFace = GfxFrontFace::CW;
		i32 depthBias = 0;
		f32 depthBiasClamp = 0.0f;
		f32 slopeScaledDepthBias = 0.0f;
	};

	struct GfxStencilFace
	{
		GfxStencilOp failOp = GfxStencilOp::Keep;
		GfxStencilOp depthFailOp = GfxStencilOp::Keep;
		GfxStencilOp passOp = GfxStencilOp::Keep;
		GfxCompareOp compareOp = GfxCompareOp::Always;
	};

	struct GfxDepthStencilState
	{
		b8 depthTest = false;
		b8 depthWrite = false;
		GfxCompareOp depthCompare = GfxCompareOp::Less;
		b8 stencilTest = false;
		u8 stencilReadMask = 0xFF;
		u8 stencilWriteMask = 0xFF;
		GfxStencilFace front;
		GfxStencilFace back;
	};

	struct GfxGraphicsPipelineDesc
	{
		GfxShaderBlob taskShader, meshShader, vertexShader, pixelShader;

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

	struct GfxComputePipelineDesc
	{
		GfxShaderBlob computeShader;
	};

	class GfxPipeline : public GfxObject
	{
	public:
		GfxPipelineType GetType() const { return m_type; }

	protected:
		GfxPipelineType m_type = GfxPipelineType::Graphics;
	};
}