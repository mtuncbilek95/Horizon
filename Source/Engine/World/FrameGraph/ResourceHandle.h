#pragma once

#include <limits>

namespace Horizon
{
	template<typename Tag>
	struct ResourceHandle
	{
		u32 idx = 0;
		u32 version = 0;

		b8 isValid() const { return idx != u32_max; }
		b8 operator==(const ResourceHandle& other) { return idx == other.idx; }
		b8 operator!=(const ResourceHandle& other) { return idx != other.idx; }
	};

	struct RenderTargetTag;
	struct RenderBufferTag;

	using RenderTargetHandle = ResourceHandle<RenderTargetTag>;
	using RenderBufferHandle = ResourceHandle<RenderBufferTag>;
}