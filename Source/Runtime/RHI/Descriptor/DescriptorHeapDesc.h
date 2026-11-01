#pragma once

#include <Runtime/RHI/Descriptor/DescriptorHeapType.h>

namespace Horizon::RHI
{
	struct DescriptorHeapDesc final
	{
		DescriptorHeapType type = DescriptorHeapType::Resource;
		u32 capacity = 0;
	};
}