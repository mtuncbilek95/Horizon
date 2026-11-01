#pragma once

#include <Runtime/RHI/Buffer/BufferUsage.h>
#include <Runtime/RHI/Memory/MemoryType.h>

namespace Horizon::RHI
{
	struct BufferDesc final
	{
		u64 size = 0;
		BufferUsage usage = BufferUsage::None;
		MemoryType memory = MemoryType::GpuOnly;
	};
}