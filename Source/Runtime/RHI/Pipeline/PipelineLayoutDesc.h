#pragma once

#include <Runtime/RHI/Descriptor/DescriptorTableLayout.h>

namespace Horizon::RHI
{
	struct PipelineLayoutDesc final
	{
		List<DescriptorTableLayout> tables;
		u32 pushConstantSize = 0;
		b8 bindless = false;
	};
}