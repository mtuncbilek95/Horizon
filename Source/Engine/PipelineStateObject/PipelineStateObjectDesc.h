#pragma once

#include <Runtime/Graphics/RHI/Pipeline/GfxPipelineDesc.h>
#include <Engine/PipelineStateObject/BlendMode.h>

#include <unordered_map>

namespace Horizon
{
	struct PipelineStateObjectDesc
	{
		std::string pipeName;

		std::unordered_map<ShaderStage, std::filesystem::path> shaders;

		PrimitiveTopology topo = PrimitiveTopology::TriangleList;
		CullMode cullMode = CullMode::Back;
		PolygonMode fillMode = PolygonMode::Fill;

		f32 depthConst = 0.f;
		f32 depthSlope = 0.f;

		b8 depthTest = false;
		b8 depthWrite = false;
		CompareOp depthOp = CompareOp::Less;
		BlendMode::Values blendMode = BlendMode::Values::Opaque;

		std::vector<ImageFormat> colorFormats;
		ImageFormat depthFormat;

		PipelineStateObjectDesc& setName(std::string_view name) { pipeName = name; return *this; }
		PipelineStateObjectDesc& setShader(ShaderStage stage, std::filesystem::path pt) { shaders[stage] = std::move(pt); return *this; }
		PipelineStateObjectDesc& setTopology(PrimitiveTopology t) { topo = t; return *this; }
		PipelineStateObjectDesc& setCullMode(CullMode mode) { cullMode = mode; return *this; }
		PipelineStateObjectDesc& setPolygonMode(PolygonMode mode) { fillMode = mode; return *this; }
		PipelineStateObjectDesc& setDepthBiasConstant(f32 val) { depthConst = val; return *this; }
		PipelineStateObjectDesc& setDepthBiasSlope(f32 val) { depthSlope = val; return *this; }
		PipelineStateObjectDesc& setDepthTest(b8 enable) { depthTest = enable; return *this; }
		PipelineStateObjectDesc& setDepthWrite(b8 enable) { depthWrite = enable; return *this; }
		PipelineStateObjectDesc& setDepthOp(CompareOp op) { depthOp = op; return *this; }
		PipelineStateObjectDesc& setBlendMode(BlendMode::Values mode) { blendMode = mode; return *this; }
		PipelineStateObjectDesc& addColorFormat(ImageFormat fmt) { colorFormats.push_back(fmt); return *this; }
		PipelineStateObjectDesc& setDepthFormat(ImageFormat fmt) { depthFormat = fmt; return *this; }
	};
}