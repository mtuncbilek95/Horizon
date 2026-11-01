#pragma once

#include <Runtime/RHI/Common/SampleCount.h>
#include <Runtime/RHI/Pipeline/ColorTarget.h>
#include <Runtime/RHI/Pipeline/DepthStencilState.h>
#include <Runtime/RHI/Pipeline/PrimitiveTopology.h>
#include <Runtime/RHI/Pipeline/RasterizerState.h>
#include <Runtime/RHI/Pipeline/VertexLayout.h>

namespace Horizon::RHI
{
	class PipelineLayout;
	class ShaderModule;

	struct GraphicsPipelineDesc final
	{
		PipelineLayout* pLayout = nullptr;

		ShaderModule* pVertexShader = nullptr;
		ShaderModule* pPixelShader = nullptr;
		ShaderModule* pTaskShader = nullptr;
		ShaderModule* pMeshShader = nullptr;

		VertexLayout vertexLayout;
		PrimitiveTopology topology = PrimitiveTopology::TriangleList;

		RasterizerState rasterizer;
		DepthStencilState depthStencil;

		List<ColorTarget> colorTargets;
		TextureFormat depthFormat = TextureFormat::Undefined;

		SampleCount sampleCount = SampleCount::Count1;
		b8 alphaToCoverage = false;
	};
}
