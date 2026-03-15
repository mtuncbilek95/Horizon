#pragma once

#include <Runtime/Graphics/RHI/Pipeline/GfxPipelineDesc.h>

#include <memory>

namespace Horizon
{
	struct GraphicsPipelineStateDesc
	{
		DynamicRendering rendering;

		u32 shaderIds[8];

		CullMode cullMode;
		PolygonMode fillMode;

		bool depthTestEnable;
		bool depthWriteEnable;
		CompareOp depthCompareOp;

		bool blendEnable;

		bool operator==(const GraphicsPipelineStateDesc& desc) const
		{ 
			return std::memcmp(this, &desc, sizeof(GraphicsPipelineStateDesc)) == 0;
		}
	};
}