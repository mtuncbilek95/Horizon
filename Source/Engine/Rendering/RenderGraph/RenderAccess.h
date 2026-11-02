#pragma once

#include <Runtime/RHI/Command/ResourceState.h>

namespace Horizon::Engine
{
	struct RenderResource final
	{
		u32 index = kInvalid32;
		u32 version = 0;
	};

	struct RenderAccess final
	{
		RenderResource resource;
		RHI::ResourceState state = RHI::ResourceState::Undefined;
	};
}