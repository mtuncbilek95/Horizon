#include "DX12Context.h"

namespace Horizon
{
	GfxPipelineHandle GfxDevice::CreatePipeline(const GfxGraphicsPipelineDesc& desc)
	{
		return GfxPipelineHandle();
	}

	GfxPipelineHandle GfxDevice::CreatePipeline(const GfxComputePipelineDesc& desc)
	{
		return GfxPipelineHandle();
	}

	void GfxDevice::DestroyPipeline(GfxPipelineHandle handle)
	{}
}