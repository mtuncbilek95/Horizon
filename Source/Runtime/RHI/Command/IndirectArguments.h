#pragma once

#include <Runtime/Definitions/PrimitiveDefinitions.h>

namespace Horizon::RHI
{
	struct DrawArguments final
	{
		u32 vertexCount = 0;
		u32 instanceCount = 0;
		u32 firstVertex = 0;
		u32 firstInstance = 0;
	};

	struct DrawIndexedArguments final
	{
		u32 indexCount = 0;
		u32 instanceCount = 0;
		u32 firstIndex = 0;
		i32 vertexOffset = 0;
		u32 firstInstance = 0;
	};

	struct DispatchArguments final
	{
		u32 groupX = 0;
		u32 groupY = 0;
		u32 groupZ = 0;
	};
}
