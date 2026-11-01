#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	class DescriptorHeap;
	class PipelineLayout;

	struct DescriptorTableDesc final
	{
		DescriptorHeap* pHeap = nullptr;
		PipelineLayout* pLayout = nullptr;
		u32 tableIndex = 0;
		u32 variableCount = 0;
	};
}