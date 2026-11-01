#include "Device.h"

#include <Runtime/RHI/Pipeline/GlobalPipelineLayout.h>
#include <Runtime/RHI/Pipeline/PipelineLayout.h>

namespace Horizon::RHI
{
	UniquePtr<PipelineLayout> Device::CreateRHIGlobalPipelineLayout()
	{
		PipelineLayoutDesc desc = {};
		desc.pushConstantSize = kGlobalPushConstantSize;
		desc.bindless = true;

		return CreateRHIPipelineLayout(desc);
	}
}