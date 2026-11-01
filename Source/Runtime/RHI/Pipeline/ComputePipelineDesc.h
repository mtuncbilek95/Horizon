#pragma once

namespace Horizon::RHI
{
	class PipelineLayout;
	class ShaderModule;

	struct ComputePipelineDesc final
	{
		PipelineLayout* pLayout = nullptr;
		ShaderModule* pComputeShader = nullptr;
	};
}
