#pragma once

#include <Runtime/RHI/Command/ResourceState.h>

namespace Horizon::RHI
{
	class Buffer;

	struct BufferBarrier final
	{
		Buffer* pBuffer = nullptr;
		ResourceState before = ResourceState::Undefined;
		ResourceState after = ResourceState::Undefined;
	};
}
