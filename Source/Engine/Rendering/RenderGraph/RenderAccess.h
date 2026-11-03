#pragma once

#include <Runtime/RHI/Command/ResourceState.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::Engine
{
	struct RenderAccess final
	{
		Reflect::TypeHandle resource;
		RHI::ResourceState state = RHI::ResourceState::Undefined;
	};
}