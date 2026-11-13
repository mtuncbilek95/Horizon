#pragma once

#include <Engine/World/ECS/ComponentIdAttribute.h>
#include <Engine/World/ECS/ComponentObject.h>
#include <Runtime/RTTR/Reflection.h>

namespace Horizon::World::Component
{
	HCLASS(Engine::ComponentId["PhysicsBody", "Physics"]);
	class PhysicsBody final : public Engine::ComponentObject
	{
		HORIZON_TYPE_REFLECT(PhysicsBody);
	public:
		PhysicsBody() = default;
		~PhysicsBody() = default;

		u32 m_bodyId = kInvalid32;
		u64 m_settingsHash = 0;
		u64 m_shapeHash = 0;
	};
}