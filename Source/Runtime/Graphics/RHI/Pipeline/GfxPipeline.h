#pragma once

#include <Runtime/Graphics/RHI/GfxTypes.h>
#include <Runtime/Graphics/RHI/Object/GfxResource.h>

namespace Horizon
{
	struct GfxShaderBlob 
	{ 
		const void* pData = nullptr; 
		usize size = 0; 
	};

	struct GfxGraphicsPipelineDesc
	{
		GfxShaderBlob taskShader, meshShader, vertexShader, computeShader, pixelShader;
		GfxTextureFormat colorFormats[8] = {};
		u32 colorTargetCount = 0;
		GfxTextureFormat depthFormat = GfxTextureFormat::Undefined;
		GfxPrimitiveTopology topology = GfxPrimitiveTopology::TriangleList;
		GfxCullMode cullMode = GfxCullMode::Back;
		GfxFrontFace frontFace = GfxFrontFace::CW;
		GfxFillMode fillMode = GfxFillMode::Solid;
		b8 depthTest = false, depthWrite = false;
		GfxCompareOp depthCompare = GfxCompareOp::Less;
		i32 depthBias = 0; f32 slopeScaledDepthBias = 0.0f;
		GfxBlendState blend;
	};

	struct GfxComputePipelineDesc 
	{ 
		GfxShaderBlob computeShader; 
	};

	class GfxPipeline : public GfxResource
	{
	public:
		GfxPipelineType GetType() const { return m_type; }

	protected:
		GfxPipelineType m_type = GfxPipelineType::Graphics;
	};
}