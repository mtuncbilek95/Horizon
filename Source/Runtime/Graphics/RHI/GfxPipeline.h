#pragma once

#include <Runtime/Graphics/RHI/GfxObject.h>

#include <array>

namespace Horizon
{
	struct GfxShaderBytecode
	{
		const void* data = nullptr;
		usize size = 0;
	};

	struct GfxGraphicsPipelineDesc
	{
		GfxShaderBytecode vertex, pixel, mesh, task, geom;

		GfxPrimitiveTopology topology = GfxPrimitiveTopology::TriangleList;
		GfxFillMode fill = GfxFillMode::Solid;
		GfxCullMode cull = GfxCullMode::Back;
		GfxFrontFace frontFace = GfxFrontFace::CCW;

		b8 depthTest = true;
		b8 depthWrite = true;
		GfxCompareOp depthCompare = GfxCompareOp::GreaterEqual;

		b8 blendEnable = false;
		GfxBlendFactor srcColor = GfxBlendFactor::One;
		GfxBlendFactor dstColor = GfxBlendFactor::Zero;
		GfxBlendOp colorOp = GfxBlendOp::Add;

		u32 colorCount = 1;
		std::array<GfxTextureFormat, 8> colorFormats = { GfxTextureFormat::RGBA8 };
		GfxTextureFormat depthFormat = GfxTextureFormat::Undefined;

		const char* debugName = nullptr;
	};

	struct GfxComputePipelineDesc
	{
		GfxShaderBytecode compute;
		const char* debugName = nullptr;
	};

	class GfxPipeline : public GfxObject
	{
	public:
		GfxPipeline(const GfxGraphicsPipelineDesc& desc, GfxDevice* pDevice);
		GfxPipeline(const GfxComputePipelineDesc& desc, GfxDevice* pDevice);
		~GfxPipeline();

		GfxPipelineType GetType() const { return m_type; }

	private:
		GfxPipelineType m_type = GfxPipelineType::Graphics;
	};
}
