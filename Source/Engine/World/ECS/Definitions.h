#pragma once

#include <Runtime/Definitions/Handle.h>
#include <Runtime/RTTR/Reflection.h>

#include <bitset>

namespace Horizon::Engine
{
	static constexpr u32 AtLeastEntities = 4096;
	static constexpr u32 MaxComponents = 256;

	struct EntityTag {};
	using EntityHandle = HandleEx<EntityTag>;

	using ComponentTypeId = Reflect::TypeHandle;
	using Signature = std::bitset<MaxComponents>;
}