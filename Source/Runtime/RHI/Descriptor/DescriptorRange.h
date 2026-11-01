#pragma once

#include <Runtime/RHI/Descriptor/DescriptorType.h>

namespace Horizon::RHI
{
	struct DescriptorRange final
	{
		DescriptorType type = DescriptorType::Texture;
		u32 binding = 0;
		u32 count = 1;
	};
}