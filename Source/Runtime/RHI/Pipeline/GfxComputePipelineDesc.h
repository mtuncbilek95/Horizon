#pragma once

namespace Horizon::RHI
{
	class GfxShader;

	struct GfxComputePipelineDesc
	{
		GfxShader* pComputeShader = nullptr;
	};
}
