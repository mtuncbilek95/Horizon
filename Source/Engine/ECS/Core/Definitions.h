#pragma once

#include <cstdint>
#include <cassert>
#include <bitset>
#include <atomic>
#include <vector>
#include <array>

namespace Horizon
{
	static constexpr u32 MaxEntities = 4096;
	static constexpr u64 MaxComponents = 64;

	struct EntityTag {};
	using EntityHandle = Handle<EntityTag>;

	struct ComponentTag {};
	using ComponentHandle = Handle<ComponentTag>;

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