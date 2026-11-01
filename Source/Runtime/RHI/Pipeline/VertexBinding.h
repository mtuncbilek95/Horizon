#pragma once

#include <Runtime/RHI/Pipeline/VertexInputRate.h>

namespace Horizon::RHI
{
	struct VertexBinding final
	{
		u32 binding = 0;
		u32 stride = 0;
		VertexInputRate inputRate = VertexInputRate::PerVertex;
	};
}
