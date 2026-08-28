#pragma once

namespace Horizon::RHI
{
	class GfxShader;

	/**
	 * @brief Creation descriptor of a compute pipeline,
	 * which needs nothing beyond the compute shader since
	 * the resources are reached bindlessly.
	 *
	 * @code
	 *   RHI::GfxComputePipelineDesc pipeDesc = {};
	 *   pipeDesc.pComputeShader = myComputeShader;
	 *   RHI::GfxPipeline* pipeline = myRhiDevice->CreatePipeline(pipeDesc);
	 * @endcode
	 */
	struct GfxComputePipelineDesc
	{
		GfxShader* pComputeShader = nullptr;
	};
}
