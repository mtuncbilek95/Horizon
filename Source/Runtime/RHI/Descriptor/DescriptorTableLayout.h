#pragma once

#include <Runtime/Containers/Vector/List.h>
#include <Runtime/RHI/Descriptor/DescriptorRange.h>

namespace Horizon::RHI
{
	struct DescriptorTableLayout final
	{
		List<DescriptorRange> ranges;
	};
}