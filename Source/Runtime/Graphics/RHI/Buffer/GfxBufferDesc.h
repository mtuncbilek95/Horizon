#pragma once

#include <Runtime/Graphics/RHI/Util/BufferFormat.h>
#include <Runtime/Graphics/RHI/Util/MemoryFormat.h>

namespace Horizon
{
	struct GfxBufferDesc
	{
		usize sizeInBytes = 0;
		BufferUsage usage = BufferUsage::Uniform;

		MemoryUsage memUsage = MemoryUsage::AutoPreferHost;
		MemoryAllocation allocFlags = MemoryAllocation::Mapped;

		GfxBufferDesc& setSize(usize size) { sizeInBytes = size; return *this; }
		GfxBufferDesc& setUsage(BufferUsage u) { usage = u; return *this; }
		GfxBufferDesc& setMemoryUsage(MemoryUsage mem) { memUsage = mem; return *this; }
		GfxBufferDesc& setAllocationFlags(MemoryAllocation flags) { allocFlags = flags; return *this; }
	};
}
