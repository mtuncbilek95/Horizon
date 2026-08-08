#pragma once

#include <Runtime/Definitions/Handle.h>
#include <atomic>
#include <bitset>

namespace Horizon::Engine
{
	static constexpr u32 MaxEntities = 8192;
	static constexpr u64 MaxComponents = 64;

	struct EntityTag {};
	using EntityHandle = HandleEx<EntityTag>;

	struct ComponentTag {};
	using ComponentHandle = HandleEx<ComponentTag>;

	using EntityId = u32;
	using ComponentTypeId = u64;
	using Signature = std::bitset<MaxComponents>;

	inline ComponentTypeId GetUniqueComponentId()
	{
		static std::atomic<ComponentTypeId> lastId = 0u;
		return lastId++;
	}

	template<typename T>
	inline ComponentTypeId GetComponentTypeId()
	{
		static const ComponentTypeId typeId = GetUniqueComponentId();
		return typeId;
	}
}