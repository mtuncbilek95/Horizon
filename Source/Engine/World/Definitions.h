#pragma once

#include <Runtime/Definitions/Handle.h>
#include <Runtime/Definitions/PrimitiveDefinitions.h>
#include <Runtime/RTTR/TypeHandle.h>

#include <bitset>

namespace Horizon::Engine
{
	static constexpr u32 MaxEntities = 8192;
	static constexpr u32 MaxComponents = 64;
	static constexpr u32 InvalidSlot = kInvalid32;
	static constexpr u32 InvalidDenseIndex = kInvalid32;

	struct EntityTag {};
	using EntityHandle = HandleEx<EntityTag>;

	using ComponentTypeId = Reflect::TypeHandle;
	using Signature = std::bitset<MaxComponents>;
}